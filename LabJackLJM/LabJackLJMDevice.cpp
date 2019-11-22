//
//  LabJackLJMDevice.cpp
//  LabJackLJM
//
//  Created by Christopher Stawarz on 11/20/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#include "LabJackLJMDevice.hpp"


BEGIN_NAMESPACE_MW_LABJACK_LJM


const std::string Device::DEVICE_TYPE("device_type");
const std::string Device::CONNECTION_TYPE("connection_type");
const std::string Device::IDENTIFIER("identifier");
const std::string Device::DATA_INTERVAL("data_interval");
const std::string Device::UPDATE_INTERVAL("update_interval");


void Device::describeComponent(ComponentInfo &info) {
    IODevice::describeComponent(info);
    
    info.setSignature("iodevice/labjack_ljm");
    
    info.addParameter(DEVICE_TYPE, "ANY");
    info.addParameter(CONNECTION_TYPE, "ANY");
    info.addParameter(IDENTIFIER, "ANY");
    info.addParameter(DATA_INTERVAL);
    info.addParameter(UPDATE_INTERVAL);
}


Device::Device(const ParameterValueMap &parameters) :
    IODevice(parameters),
    deviceType(variableOrText(parameters[DEVICE_TYPE])->getValue().getString()),
    connectionType(variableOrText(parameters[CONNECTION_TYPE])->getValue().getString()),
    identifier(variableOrText(parameters[IDENTIFIER])->getValue().getString()),
    dataInterval(parameters[DATA_INTERVAL]),
    updateInterval(parameters[UPDATE_INTERVAL]),
    handle(-1),
    running(false)
{
    if (dataInterval <= 0) {
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN, "Invalid data interval");
    }
    if (updateInterval <= 0) {
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN, "Invalid update interval");
    }
    if (updateInterval % dataInterval != 0) {
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN,
                              "Update interval must be an integer multiple of data interval");
    }
}


Device::~Device() {
    if (-1 != handle) {
        logError(LJM_Close(handle), "Cannot close LJM device");
    }
}


void Device::addChild(std::map<std::string, std::string> parameters,
                      ComponentRegistryPtr reg,
                      boost::shared_ptr<Component> child)
{
    if (auto channel = boost::dynamic_pointer_cast<SingleLineChannel>(child)) {
        reserveLine(channel->getLineName(), channel->getLine());
        if (auto diChannel = boost::dynamic_pointer_cast<DigitalInputChannel>(channel)) {
            digitalInputChannels.emplace_back(std::move(diChannel));
            return;
        }
        if (auto doChannel = boost::dynamic_pointer_cast<DigitalOutputChannel>(channel)) {
            digitalOutputChannels.emplace_back(std::move(doChannel));
            return;
        }
    }
    throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN, "Invalid channel type for LabJack LJM device");
}


bool Device::initialize() {
    if (logError(LJM_OpenS(deviceType.c_str(), connectionType.c_str(), identifier.c_str(), &handle),
                 "Cannot open LJM device"))
    {
        return false;
    }
    
    writeBuffer.append("IO_CONFIG_SET_CURRENT_TO_FACTORY", 1);
    prepareDigitalInput();
    prepareDigitalOutput();
    
    if (logError(writeBuffer.write(handle), "Cannot configure LJM device")) {
        return false;
    }
    
    return true;
}


bool Device::startDeviceIO() {
    lock_guard lock(mutex);
    
    if (!running) {
        updateDigitalOutput(true);
        
        if (logError(writeBuffer.write(handle), "Cannot start LJM device")) {
            return false;
        }
        
        running = true;
    }
    
    return true;
}


bool Device::stopDeviceIO() {
    lock_guard lock(mutex);
    
    if (running) {
        updateDigitalOutput();
        
        if (logError(writeBuffer.write(handle), "Cannot stop LJM device")) {
            return false;
        }
        
        running = false;
    }
    
    return true;
}


int Device::convertNameToAddress(const std::string &name, int &type) {
    int address;
    throwError(LJM_NameToAddress(name.c_str(), &address, &type),
               "Internal error: Cannot convert LJM register name to address");
    return address;
}


void Device::reserveLine(const std::string &lineName, PhysicalLine line) {
    if (!(linesInUse.insert(line).second)) {
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN, boost::format("Line %s is already in use") % lineName);
    }
}


void Device::prepareDigitalInput() {
    auto dioInhibit = ~std::uint32_t(0);
    
    for (auto &channel : digitalInputChannels) {
        dioInhibit ^= (1 << channel->getDIOIndex());
    }
    
    if (~dioInhibit) {
        writeBuffer.append("DIO_INHIBIT", dioInhibit);
        writeBuffer.append("DIO_DIRECTION", 0);
        stream.add("FIO_EIO_STATE");
        stream.add("CIO_MIO_STATE");
    }
}


void Device::prepareDigitalOutput() {
    boost::weak_ptr<Device> weakThis(component_shared_from_this<Device>());
    
    for (auto &channel : digitalOutputChannels) {
        int type;
        auto address = convertNameToAddress((boost::format("DIO%d") % channel->getDIOIndex()).str(), type);
        auto callback = [weakThis, address, type](const Datum &data, MWTime time) {
            if (auto sharedThis = weakThis.lock()) {
                lock_guard lock(sharedThis->mutex);
                if (sharedThis->running) {
                    logError(LJM_eWriteAddress(sharedThis->handle, address, type, data.getBool()),
                             "Cannot set digital output line on LJM device");
                }
            }
        };
        channel->getValueVariable()->addNotification(boost::make_shared<VariableCallbackNotification>(callback));
    }
    
    updateDigitalOutput();
}


void Device::updateDigitalOutput(bool active) {
    auto dioInhibit = ~std::uint32_t(0);
    auto dioDirection = std::uint32_t(0);
    auto dioState = std::uint32_t(0);
    
    for (auto &channel : digitalOutputChannels) {
        auto dioIndex = channel->getDIOIndex();
        dioInhibit ^= (1 << dioIndex);
        dioDirection |= (1 << dioIndex);
        if (active) {
            dioState |= (channel->getValueVariable()->getValue().getBool() << dioIndex);
        }
    }
    
    if (~dioInhibit) {
        writeBuffer.append("DIO_INHIBIT", dioInhibit);
        writeBuffer.append("DIO_DIRECTION", dioDirection);
        writeBuffer.append("DIO_STATE", dioState);
    }
}


void Device::WriteBuffer::append(const std::string &name, double value) {
    int type;
    auto address = convertNameToAddress(name, type);
    addresses.push_back(address);
    types.push_back(type);
    values.push_back(value);
}


int Device::WriteBuffer::write(int handle) {
    if (addresses.empty()) {
        return LJME_NOERROR;
    }
    int errorAddress = -1;
    auto result = LJM_eWriteAddresses(handle,
                                      addresses.size(),
                                      addresses.data(),
                                      types.data(),
                                      values.data(),
                                      &errorAddress);
    addresses.clear();
    types.clear();
    values.clear();
    return result;
}


void Device::Stream::add(const std::string &name) {
    addresses.push_back(convertNameToAddress(name));
}


END_NAMESPACE_MW_LABJACK_LJM
