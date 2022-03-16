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
const std::string Device::UPDATE_INTERVAL("update_interval");


void Device::describeComponent(ComponentInfo &info) {
    IODevice::describeComponent(info);
    
    info.setSignature("iodevice/labjack_ljm");
    
    info.addParameter(DEVICE_TYPE, "ANY");
    info.addParameter(CONNECTION_TYPE, "ANY");
    info.addParameter(IDENTIFIER, "ANY");
    info.addParameter(UPDATE_INTERVAL);
}


Device::Device(const ParameterValueMap &parameters) :
    IODevice(parameters),
    deviceType(variableOrText(parameters[DEVICE_TYPE])->getValue().getString()),
    connectionType(variableOrText(parameters[CONNECTION_TYPE])->getValue().getString()),
    identifier(variableOrText(parameters[IDENTIFIER])->getValue().getString()),
    updateInterval(parameters[UPDATE_INTERVAL]),
    clock(Clock::instance()),
    handle(-1),
    running(false)
{
    if (updateInterval <= 0) {
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN, "Invalid update interval");
    }
}


Device::~Device() {
    if (-1 != handle) {
        logError(LJM_eWriteName(handle, "LED_COMM", 0), "Cannot turn off LJM device COMM LED");
        logError(LJM_Close(handle), "Cannot close LJM device");
    }
}


void Device::addChild(std::map<std::string, std::string> parameters,
                      ComponentRegistryPtr reg,
                      boost::shared_ptr<Component> child)
{
    if (auto channel = boost::dynamic_pointer_cast<DigitalInputChannel>(child)) {
        digitalInputChannels.emplace_back(std::move(channel));
        return;
    }
    if (auto channel = boost::dynamic_pointer_cast<DigitalOutputChannel>(child)) {
        digitalOutputChannels.emplace_back(std::move(channel));
        return;
    }
    throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN, "Invalid channel type for LabJack LJM device");
}


bool Device::initialize() {
    if (logError(LJM_OpenS(deviceType.c_str(), connectionType.c_str(), identifier.c_str(), &handle),
                 "Cannot open LJM device"))
    {
        return false;
    }
    
    int actualDeviceType;
    if (logError(LJM_GetHandleInfo(handle, &actualDeviceType, nullptr, nullptr, nullptr, nullptr, nullptr),
                 "Cannot determine type of opened LJM device"))
    {
        return false;
    }
    deviceInfo = DeviceInfo::getDeviceInfo(actualDeviceType);
    if (!deviceInfo) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "Selected LJM device is not supported (type = %d)", actualDeviceType);
        return false;
    }
    
    WriteBuffer writeBuffer;
    
    writeBuffer.append("IO_CONFIG_SET_CURRENT_TO_FACTORY", 1);
    writeBuffer.append("POWER_LED", 4);  // Set LED operation to manual
    writeBuffer.append("LED_STATUS", 1);
    writeBuffer.append("LED_COMM", 1);
    
    if (haveDigitalInputs()) {
        prepareDigitalInputs(writeBuffer);
    }
    if (haveDigitalOutputs()) {
        prepareDigitalOutputs(writeBuffer);
    }
    
    if (logError(writeBuffer.write(handle), "Cannot configure LJM device")) {
        return false;
    }
    
    return true;
}


bool Device::startDeviceIO() {
    lock_guard lock(mutex);
    
    if (!running) {
        WriteBuffer writeBuffer;
        
        if (haveDigitalOutputs()) {
            updateDigitalOutputs(writeBuffer, true);
        }
        
        if (logError(writeBuffer.write(handle), "Cannot start LJM device")) {
            return false;
        }
        
        if (haveInputs() && !readInputsTask) {
            startReadInputsTask();
        }
        
        running = true;
    }
    
    return true;
}


bool Device::stopDeviceIO() {
    lock_guard lock(mutex);
    
    if (running) {
        if (readInputsTask) {
            stopReadInputsTask();
        }
        
        WriteBuffer writeBuffer;
        
        if (haveDigitalOutputs()) {
            updateDigitalOutputs(writeBuffer);
        }
        
        if (logError(writeBuffer.write(handle), "Cannot stop LJM device")) {
            return false;
        }
        
        running = false;
    }
    
    return true;
}


void Device::IOBuffer::append(const std::string &name, double value) {
    int type;
    auto address = convertNameToAddress(name, type);
    names.emplace_back(name);
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
    if (LJME_NOERROR != result) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "Error writing to LJM register \"%s\"", names.at(errorAddress).c_str());
    }
    
    return result;
}


inline double Device::ReadBuffer::ValueReader::next() {
    if (iter == end) {
        throw std::out_of_range("Internal error: no more values to read");
    }
    return *(iter++);
}


int Device::ReadBuffer::read(int handle) {
    if (addresses.empty()) {
        return LJME_NOERROR;
    }
    
    int errorAddress = -1;
    auto result = LJM_eReadAddresses(handle,
                                     addresses.size(),
                                     addresses.data(),
                                     types.data(),
                                     values.data(),
                                     &errorAddress);
    if (LJME_NOERROR != result) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "Error reading from LJM register \"%s\"", names.at(errorAddress).c_str());
    }
    
    return result;
}


int Device::convertNameToAddress(const std::string &name, int &type) {
    int address;
    throwError(LJM_NameToAddress(name.c_str(), &address, &type),
               "Internal error: Cannot convert LJM register name to address");
    return address;
}


int Device::reserveLine(const std::string &lineName) {
    const auto line = deviceInfo->getLineForName(lineName);
    if (!(linesInUse.insert(line).second)) {
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN, boost::format("Line %s is already in use") % lineName);
    }
    return line;
}


void Device::validateDigitalChannel(const boost::shared_ptr<DigitalChannel> &channel) {
    auto &lineName = channel->getLineName();
    const auto line = reserveLine(lineName);
    if (!(deviceInfo->isDIO(line))) {
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN, boost::format("%s is not a digital line") % lineName);
    }
    channel->setDIOIndex(deviceInfo->getDIOIndex(line));
}


void Device::prepareDigitalInputs(WriteBuffer &writeBuffer) {
    auto dioInhibit = ~std::uint32_t(0);
    
    for (auto &channel : digitalInputChannels) {
        validateDigitalChannel(channel);
        dioInhibit ^= (1 << channel->getDIOIndex());
    }
    
    writeBuffer.append("DIO_INHIBIT", dioInhibit);
    if (deviceInfo->hasFlexibleIO()) {
        writeBuffer.append("DIO_ANALOG_ENABLE", 0);
    }
    writeBuffer.append("DIO_DIRECTION", 0);
    
    readBuffer.append("DIO_STATE");
}


void Device::prepareDigitalOutputs(WriteBuffer &writeBuffer) {
    boost::weak_ptr<Device> weakThis(component_shared_from_this<Device>());
    
    for (auto &channel : digitalOutputChannels) {
        validateDigitalChannel(channel);
        int type;
        auto address = convertNameToAddress((boost::format("DIO%d") % channel->getDIOIndex()).str(), type);
        // It's OK to capture channel by reference, because it will remain alive (in
        // digitalOutputChannels) for as long as the device is alive
        auto callback = [weakThis, address, type, &channel](const Datum &data, MWTime time) {
            if (auto sharedThis = weakThis.lock()) {
                lock_guard lock(sharedThis->mutex);
                if (sharedThis->running) {
                    logError(LJM_eWriteAddress(sharedThis->handle, address, type, channel->getValue()),
                             "Cannot set digital output line on LJM device");
                }
            }
        };
        channel->addNewValueNotification(boost::make_shared<VariableCallbackNotification>(callback));
    }
    
    updateDigitalOutputs(writeBuffer);
}


void Device::updateDigitalOutputs(WriteBuffer &writeBuffer, bool active) {
    auto dioInhibit = ~std::uint32_t(0);
    auto dioDirection = std::uint32_t(0);
    auto dioState = std::uint32_t(0);
    
    for (auto &channel : digitalOutputChannels) {
        auto dioIndex = channel->getDIOIndex();
        dioInhibit ^= (1 << dioIndex);
        dioDirection |= (1 << dioIndex);
        if (active) {
            dioState |= (channel->getValue() << dioIndex);
        }
    }
    
    writeBuffer.append("DIO_INHIBIT", dioInhibit);
    if (deviceInfo->hasFlexibleIO()) {
        writeBuffer.append("DIO_ANALOG_ENABLE", 0);
    }
    writeBuffer.append("DIO_DIRECTION", dioDirection);
    writeBuffer.append("DIO_STATE", dioState);
}


void Device::startReadInputsTask() {
    boost::weak_ptr<Device> weakThis(component_shared_from_this<Device>());
    readInputsTask = Scheduler::instance()->scheduleUS(FILELINE,
                                                       updateInterval,
                                                       updateInterval,
                                                       M_REPEAT_INDEFINITELY,
                                                       [weakThis]() {
                                                           if (auto sharedThis = weakThis.lock()) {
                                                               lock_guard lock(sharedThis->mutex);
                                                               sharedThis->readInputs();
                                                           }
                                                           return nullptr;
                                                       },
                                                       M_DEFAULT_IODEVICE_PRIORITY,
                                                       M_DEFAULT_IODEVICE_WARN_SLOP_US,
                                                       M_DEFAULT_IODEVICE_FAIL_SLOP_US,
                                                       M_MISSED_EXECUTION_DROP);
}


void Device::stopReadInputsTask() {
    readInputsTask->cancel();
    readInputsTask.reset();
}


void Device::readInputs() {
    if (!readInputsTask) {
        // If we've already been canceled, don't try to read more data
        return;
    }
    
    if (logError(readBuffer.read(handle), "Cannot read inputs from LJM device")) {
        return;
    }
    
    const auto currentTime = clock->getCurrentTimeUS();
    auto values = readBuffer.getValues();
    
    if (haveDigitalInputs()) {
        const auto dioState = std::uint32_t(values.next());
        for (auto &channel : digitalInputChannels) {
            channel->setValue(bool(dioState & (1 << channel->getDIOIndex())), currentTime);
        }
    }
}


END_NAMESPACE_MW_LABJACK_LJM
