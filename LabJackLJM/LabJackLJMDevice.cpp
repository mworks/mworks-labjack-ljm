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
    if (auto channel = boost::dynamic_pointer_cast<AnalogInputChannel>(child)) {
        analogInputChannels.emplace_back(std::move(channel));
    } else if (auto channel = boost::dynamic_pointer_cast<AnalogOutputChannel>(child)) {
        analogOutputChannels.emplace_back(std::move(channel));
    } else if (auto channel = boost::dynamic_pointer_cast<DigitalInputChannel>(child)) {
        digitalInputChannels.emplace_back(std::move(channel));
    } else if (auto channel = boost::dynamic_pointer_cast<DigitalOutputChannel>(child)) {
        digitalOutputChannels.emplace_back(std::move(channel));
    } else if (auto channel = boost::dynamic_pointer_cast<CounterChannel>(child)) {
        counterChannels.emplace_back(std::move(channel));
    } else if (auto channel = boost::dynamic_pointer_cast<QuadratureInputChannel>(child)) {
        quadratureInputChannels.emplace_back(std::move(channel));
    } else {
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN, "Invalid channel type for LabJack LJM device");
    }
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
    
    WriteBuffer configBuffer;
    
    configBuffer.append("IO_CONFIG_SET_CURRENT_TO_FACTORY", 1);
    configBuffer.append("POWER_LED", 4);  // Set LED operation to manual
    configBuffer.append("LED_STATUS", 1);
    configBuffer.append("LED_COMM", 1);
    
    if (haveAnalogInputs()) {
        prepareAnalogInputs(configBuffer);
    }
    if (haveAnalogOutputs()) {
        prepareAnalogOutputs(configBuffer);
    }
    if (haveDigitalInputs()) {
        prepareDigitalInputs(configBuffer);
    }
    if (haveDigitalOutputs()) {
        prepareDigitalOutputs(configBuffer);
    }
    if (haveCounters()) {
        prepareCounters(configBuffer);
    }
    if (haveQuadratureInputs()) {
        prepareQuadratureInputs(configBuffer);
    }
    
    if (logError(configBuffer.write(handle), "Cannot configure LJM device")) {
        return false;
    }
    
    return true;
}


bool Device::startDeviceIO() {
    lock_guard lock(mutex);
    
    if (!running) {
        WriteBuffer configBuffer;
        
        if (haveAnalogOutputs()) {
            updateAnalogOutputs(configBuffer, true);
        }
        if (haveDigitalOutputs()) {
            updateDigitalOutputs(configBuffer, true);
        }
        if (haveCounters()) {
            updateCounters(configBuffer, true);
        }
        if (haveQuadratureInputs()) {
            updateQuadratureInputs(configBuffer, true);
        }
        
        if (logError(configBuffer.write(handle), "Cannot start LJM device")) {
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
        
        WriteBuffer configBuffer;
        
        if (haveQuadratureInputs()) {
            updateQuadratureInputs(configBuffer);
        }
        if (haveCounters()) {
            updateCounters(configBuffer);
        }
        if (haveDigitalOutputs()) {
            updateDigitalOutputs(configBuffer);
        }
        if (haveAnalogOutputs()) {
            updateAnalogOutputs(configBuffer);
        }
        
        if (logError(configBuffer.write(handle), "Cannot stop LJM device")) {
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


void Device::prepareAnalogInputs(WriteBuffer &configBuffer) {
    for (auto &channel : analogInputChannels) {
        channel->resolveLine(*deviceInfo);
        inputBuffer.append(channel->getCanonicalLineName());
    }
}


void Device::prepareAnalogOutputs(WriteBuffer &configBuffer) {
    boost::weak_ptr<Device> weakThis(component_shared_from_this<Device>());
    
    for (auto &channel : analogOutputChannels) {
        channel->resolveLine(*deviceInfo);
        int type;
        auto address = convertNameToAddress(channel->getCanonicalLineName(), type);
        auto callback = [weakThis, address, type](const Datum &data, MWTime time) {
            if (auto sharedThis = weakThis.lock()) {
                lock_guard lock(sharedThis->mutex);
                if (sharedThis->running) {
                    logError(LJM_eWriteAddress(sharedThis->handle, address, type, data.getFloat()),
                             "Cannot set analog output line on LJM device");
                }
            }
        };
        channel->addNewValueNotification(boost::make_shared<VariableCallbackNotification>(callback));
    }
    
    updateAnalogOutputs(configBuffer);
}


void Device::updateAnalogOutputs(WriteBuffer &configBuffer, bool active) {
    for (auto &channel : analogOutputChannels) {
        configBuffer.append(channel->getCanonicalLineName(), (active ? channel->getValue() : 0.0));
    }
}


void Device::prepareDigitalInputs(WriteBuffer &configBuffer) {
    auto dioInhibit = ~std::uint32_t(0);
    
    for (auto &channel : digitalInputChannels) {
        channel->resolveLine(*deviceInfo);
        dioInhibit ^= (1 << channel->getDIOIndex());
    }
    
    configBuffer.append("DIO_INHIBIT", dioInhibit);
    configBuffer.append("DIO_DIRECTION", 0);
    
    inputBuffer.append("DIO_STATE");
}


void Device::prepareDigitalOutputs(WriteBuffer &configBuffer) {
    boost::weak_ptr<Device> weakThis(component_shared_from_this<Device>());
    
    for (auto &channel : digitalOutputChannels) {
        channel->resolveLine(*deviceInfo);
        int type;
        auto address = convertNameToAddress(channel->getCanonicalLineName(), type);
        auto callback = [weakThis, address, type](const Datum &data, MWTime time) {
            if (auto sharedThis = weakThis.lock()) {
                lock_guard lock(sharedThis->mutex);
                if (sharedThis->running) {
                    logError(LJM_eWriteAddress(sharedThis->handle, address, type, data.getBool()),
                             "Cannot set digital output line on LJM device");
                }
            }
        };
        channel->addNewValueNotification(boost::make_shared<VariableCallbackNotification>(callback));
    }
    
    updateDigitalOutputs(configBuffer);
}


void Device::updateDigitalOutputs(WriteBuffer &configBuffer, bool active) {
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
    
    configBuffer.append("DIO_INHIBIT", dioInhibit);
    configBuffer.append("DIO_DIRECTION", dioDirection);
    configBuffer.append("DIO_STATE", dioState);
}


void Device::prepareCounters(WriteBuffer &configBuffer) {
    for (auto &channel : counterChannels) {
        channel->resolveLine(*deviceInfo);
        auto &line = channel->getCanonicalLineName();
        
        configBuffer.append(line + "_EF_ENABLE", 0);
        configBuffer.append(line + "_EF_INDEX", 7);
        
        inputBuffer.append(line + "_EF_READ_A");
    }
}


void Device::updateCounters(WriteBuffer &configBuffer, bool active) {
    for (auto &channel : counterChannels) {
        configBuffer.append(channel->getCanonicalLineName() + "_EF_ENABLE", active);
    }
}


void Device::prepareQuadratureInputs(WriteBuffer &configBuffer) {
    for (auto &channel : quadratureInputChannels) {
        channel->resolveLines(*deviceInfo);
        auto &line1 = channel->getCanonicalFirstLineName();
        auto &line2 = channel->getCanonicalSecondLineName();
        
        configBuffer.append(line1 + "_EF_ENABLE", 0);
        configBuffer.append(line2 + "_EF_ENABLE", 0);
        configBuffer.append(line1 + "_EF_INDEX", 10);
        configBuffer.append(line2 + "_EF_INDEX", 10);
        
        inputBuffer.append(line1 + "_EF_READ_A");
    }
}


void Device::updateQuadratureInputs(WriteBuffer &configBuffer, bool active) {
    for (auto &channel : quadratureInputChannels) {
        configBuffer.append(channel->getCanonicalFirstLineName()  + "_EF_ENABLE", active);
        configBuffer.append(channel->getCanonicalSecondLineName() + "_EF_ENABLE", active);
    }
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
    
    if (logError(inputBuffer.read(handle), "Cannot read inputs from LJM device")) {
        return;
    }
    
    const auto currentTime = clock->getCurrentTimeUS();
    auto values = inputBuffer.getValues();
    
    if (haveAnalogInputs()) {
        for (auto &channel : analogInputChannels) {
            channel->setValue(values.next(), currentTime);
        }
    }
    
    if (haveDigitalInputs()) {
        const auto dioState = std::uint32_t(values.next());
        for (auto &channel : digitalInputChannels) {
            channel->setValue(bool(dioState & (1 << channel->getDIOIndex())), currentTime);
        }
    }
    
    if (haveCounters()) {
        for (auto &channel : counterChannels) {
            channel->setValue(std::uint32_t(values.next()), currentTime);
        }
    }
    
    if (haveQuadratureInputs()) {
        for (auto &channel : quadratureInputChannels) {
            //
            // The count is transmitted as an unsigned, 32-bit integer, but it's actually signed,
            // so we need to cast twice.
            //
            // Note that the result of the second cast is technically implementation-defined until
            // C++20.  See the section "Integral conversions" of the following for details:
            //
            //   https://en.cppreference.com/w/cpp/language/implicit_conversion
            //
            channel->setValue(std::int32_t(std::uint32_t(values.next())), currentTime);
        }
    }
}


END_NAMESPACE_MW_LABJACK_LJM
