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
const std::string Device::ANALOG_WAVEFORM_DATA_INTERVAL("analog_waveform_data_interval");
const std::string Device::ANALOG_WAVEFORM_TRIGGER_LINE("analog_waveform_trigger_line");
const std::string Device::READ_INPUTS_WHILE_STOPPED("read_inputs_while_stopped");


void Device::describeComponent(ComponentInfo &info) {
    IODevice::describeComponent(info);
    
    info.setSignature("iodevice/labjack_ljm");
    
    info.addParameter(DEVICE_TYPE, "ANY");
    info.addParameter(CONNECTION_TYPE, "ANY");
    info.addParameter(IDENTIFIER, "ANY");
    info.addParameter(UPDATE_INTERVAL);
    info.addParameter(ANALOG_WAVEFORM_DATA_INTERVAL, "0");
    info.addParameter(ANALOG_WAVEFORM_TRIGGER_LINE, false);
    info.addParameter(READ_INPUTS_WHILE_STOPPED, "NO");
}


Device::Device(const ParameterValueMap &parameters) :
    IODevice(parameters),
    deviceType(variableOrText(parameters[DEVICE_TYPE])->getValue().getString()),
    connectionType(variableOrText(parameters[CONNECTION_TYPE])->getValue().getString()),
    identifier(variableOrText(parameters[IDENTIFIER])->getValue().getString()),
    updateInterval(parameters[UPDATE_INTERVAL]),
    analogWaveformDataInterval(parameters[ANALOG_WAVEFORM_DATA_INTERVAL]),
    analogWaveformTriggerLine(optionalVariableOrText(parameters[ANALOG_WAVEFORM_TRIGGER_LINE])),
    readInputsWhileStopped(parameters[READ_INPUTS_WHILE_STOPPED]),
    clock(Clock::instance()),
    handle(-1),
    analogWaveformsRunning(false),
    running(false)
{
    if (updateInterval <= 0) {
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN, "Invalid update interval");
    }
}


Device::~Device() {
    if (readInputsTask) {
        stopReadInputsTask();
    }
    
    if (-1 != handle) {
        WriteBuffer configBuffer;
        if (readInputsWhileStopped) {
            if (haveQuadratureInputs()) {
                updateQuadratureInputs(configBuffer);
            }
            if (haveCounters()) {
                updateCounters(configBuffer);
            }
        }
        configBuffer.append("LED_COMM", 0);
        logError(configBuffer.write(handle), "Cannot deconfigure LabJack LJM device");
        
        logError(LJM_Close(handle), "Cannot close LabJack LJM device");
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
    } else if (auto channel = boost::dynamic_pointer_cast<AnalogWaveformChannel>(child)) {
        analogWaveformChannels.emplace_back(std::move(channel));
    } else if (auto channel = boost::dynamic_pointer_cast<DigitalInputChannel>(child)) {
        digitalInputChannels.emplace_back(std::move(channel));
    } else if (auto channel = boost::dynamic_pointer_cast<DigitalOutputChannel>(child)) {
        digitalOutputChannels.emplace_back(std::move(channel));
    } else if (auto channel = boost::dynamic_pointer_cast<WordInputChannel>(child)) {
        wordInputChannels.emplace_back(std::move(channel));
    } else if (auto channel = boost::dynamic_pointer_cast<WordOutputChannel>(child)) {
        wordOutputChannels.emplace_back(std::move(channel));
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
                 "Cannot open LabJack LJM device"))
    {
        return false;
    }
    
    int actualDeviceType;
    if (logError(LJM_GetHandleInfo(handle, &actualDeviceType, nullptr, nullptr, nullptr, nullptr, nullptr),
                 "Cannot determine type of opened LabJack LJM device"))
    {
        return false;
    }
    deviceInfo = DeviceInfo::getDeviceInfo(actualDeviceType);
    if (!deviceInfo) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "Selected LabJack LJM device is not supported (type = %d)", actualDeviceType);
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
    if (haveAnalogWaveforms()) {
        prepareAnalogWaveforms(configBuffer);
    }
    if (haveDigitalInputs()) {
        prepareDigitalInputs(configBuffer);
    }
    if (haveDigitalOutputs()) {
        prepareDigitalOutputs(configBuffer);
    }
    if (haveCounters()) {
        prepareCounters(configBuffer);
        if (readInputsWhileStopped) {
            updateCounters(configBuffer, true);
        }
    }
    if (haveQuadratureInputs()) {
        prepareQuadratureInputs(configBuffer);
        if (readInputsWhileStopped) {
            updateQuadratureInputs(configBuffer, true);
        }
    }
    
    if (logError(configBuffer.write(handle), "Cannot configure LabJack LJM device")) {
        return false;
    }
    
    if (haveInputs() && readInputsWhileStopped) {
        startReadInputsTask();
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
        if (haveCounters() && !readInputsWhileStopped) {
            updateCounters(configBuffer, true);
        }
        if (haveQuadratureInputs() && !readInputsWhileStopped) {
            updateQuadratureInputs(configBuffer, true);
        }
        
        if (logError(configBuffer.write(handle), "Cannot start LabJack LJM device")) {
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
        if (readInputsTask && !readInputsWhileStopped) {
            stopReadInputsTask();
        }
        
        WriteBuffer configBuffer;
        
        if (haveQuadratureInputs() && !readInputsWhileStopped) {
            updateQuadratureInputs(configBuffer);
        }
        if (haveCounters() && !readInputsWhileStopped) {
            updateCounters(configBuffer);
        }
        if (haveDigitalOutputs()) {
            updateDigitalOutputs(configBuffer);
        }
        if (haveAnalogWaveforms()) {
            stopAnalogWaveforms();
        }
        if (haveAnalogOutputs()) {
            updateAnalogOutputs(configBuffer);
        }
        
        if (logError(configBuffer.write(handle), "Cannot stop LabJack LJM device")) {
            return false;
        }
        
        running = false;
    }
    
    return true;
}


void Device::startAnalogWaveformOutput() {
    lock_guard lock(mutex);
    
    if (!running) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "LabJack LJM device is not running");
    } else if (!haveAnalogWaveforms()) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "LabJack LJM device has no analog waveform channels");
    } else {
        startAnalogWaveforms();
    }
}


void Device::stopAnalogWaveformOutput() {
    lock_guard lock(mutex);
    
    if (!running) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "LabJack LJM device is not running");
    } else if (!haveAnalogWaveforms()) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "LabJack LJM device has no analog waveform channels");
    } else {
        stopAnalogWaveforms();
    }
}


void Device::IOBuffer::append(const std::string &name, double value) {
    int type;
    auto address = convertNameToAddress(name, type);
    names.emplace_back(name);
    addresses.push_back(address);
    types.push_back(type);
    values.push_back(value);
}


void Device::IOBuffer::append(const IOBuffer &other) {
    append(names, other.names);
    append(addresses, other.addresses);
    append(types, other.types);
    append(values, other.values);
}


const char * Device::IOBuffer::nameForAddress(int address) const {
    auto iter = std::find(addresses.begin(), addresses.end(), address);
    if (iter != addresses.end()) {
        return names.at(std::distance(addresses.begin(), iter)).c_str();
    }
    return "UNKNOWN";
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
    if (LJME_NOERROR != result && -1 != errorAddress) {
        merror(M_IODEVICE_MESSAGE_DOMAIN,
               "Error writing to LabJack LJM register %d (%s)",
               errorAddress,
               nameForAddress(errorAddress));
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
    if (LJME_NOERROR != result && -1 != errorAddress) {
        merror(M_IODEVICE_MESSAGE_DOMAIN,
               "Error reading from LabJack LJM register %d (%s)",
               errorAddress,
               nameForAddress(errorAddress));
    }
    
    return result;
}


int Device::convertNameToAddress(const std::string &name, int &type) {
    int address;
    throwError(LJM_NameToAddress(name.c_str(), &address, &type),
               "Internal error: Cannot convert LabJack LJM register name to address");
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
                             "Cannot set analog output line on LabJack LJM device");
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


void Device::prepareAnalogWaveforms(WriteBuffer &configBuffer) {
    if (!analogWaveformTriggerLine) {
        // IO_CONFIG_SET_CURRENT_TO_FACTORY apparently does not reset STREAM_TRIGGER_INDEX,
        // so we must reset it ourselves.  If we don't, the device will be stuck in
        // triggered mode after running an experiment with a trigger line, and the user will
        // need to power cycle the device to reset it.
        if (deviceInfo->supportsHardwareTriggeredStream()) {
            configBuffer.append("STREAM_TRIGGER_INDEX", 0);
        }
    } else {
        if (!deviceInfo->supportsHardwareTriggeredStream()) {
            throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN,
                                  "LabJack LJM device does not support triggered analog waveform output");
        }
        
        const auto lineName = analogWaveformTriggerLine->getValue().getString();
        const auto line = deviceInfo->reserveLineForName(lineName);
        if (!deviceInfo->isConditionalReset(line)) {
            throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN,
                                  boost::format("%s is not a conditional reset line and cannot be used to trigger "
                                                "analog waveform output") % lineName);
        }
        const auto canonicalLineName = deviceInfo->getCanonicalLineName(line);
        
        configBuffer.append(canonicalLineName + "_EF_ENABLE", 0);
        configBuffer.append(canonicalLineName + "_EF_INDEX", 12);
        configBuffer.append(canonicalLineName + "_EF_CONFIG_A", 1);  // Rising edges
        configBuffer.append(canonicalLineName + "_EF_CONFIG_B", 1);  // One edge per reset
        // We configure the conditional reset to reset itself.  The folks at LabJack support say
        // this isn't necessary, but we do it just to be sure that no other DIO extended feature
        // is reset.
        configBuffer.append(canonicalLineName + "_EF_CONFIG_C", deviceInfo->getDIOIndex(line));
        // Enabling and disabling the conditional reset in startAnalogWaveforms and
        // stopAnalogWaveforms, respectively, seems to break triggering, so we just enable it
        // here and leave it enabled
        configBuffer.append(canonicalLineName + "_EF_ENABLE", 1);
        
        configBuffer.append("STREAM_TRIGGER_INDEX", convertNameToAddress(canonicalLineName));
    }
    
    const auto maxNumOutputStreams = deviceInfo->getMaxNumOutputStreams();
    if (analogWaveformChannels.size() > maxNumOutputStreams) {
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN,
                              (boost::format("LabJack LJM device supports at most %1% analog waveform channels")
                               % maxNumOutputStreams));
    }
    
    int streamOutIndex = 0;
    for (auto &channel : analogWaveformChannels) {
        channel->resolveLine(*deviceInfo);
        channel->setStreamOutIndex(streamOutIndex);
        channel->setStreamOutAddress(convertNameToAddress("STREAM_OUT" + std::to_string(streamOutIndex)));
        channel->setTargetAddress(convertNameToAddress(channel->getCanonicalLineName()));
        analogWaveformsResetBuffer.append(channel->getCanonicalLineName(), 0.0);
        streamOutIndex++;
    }
    
    configBuffer.append(analogWaveformsResetBuffer);
}


void Device::startAnalogWaveforms() {
    if (analogWaveformsRunning) {
        return;
    }
    
    const auto dataInterval = analogWaveformDataInterval->getValue().getInteger();
    if (dataInterval <= 0) {
        merror(M_IODEVICE_MESSAGE_DOMAIN,
               "LabJack LJM analog waveform data interval must be greater than zero");
        return;
    }
    
    const auto requestedScanRate = 1e6 / double(dataInterval);  // scans per second
    std::vector<int> addresses;
    
    for (auto &channel : analogWaveformChannels) {
        addresses.push_back(channel->getStreamOutAddress());
        
        std::vector<double> dataBuffer;
        if (!channel->getData(dataBuffer)) {
            return;
        }
        
        if (channel->getLoop()) {
            if (logError(LJM_PeriodicStreamOut(handle,
                                               channel->getStreamOutIndex(),
                                               channel->getTargetAddress(),
                                               requestedScanRate,
                                               dataBuffer.size(),
                                               dataBuffer.data()),
                         "Cannot initialize periodic analog waveform output on LabJack LJM device"))
            {
                return;
            }
        } else {
            int bufferStatus = 0;
            if (logError(LJM_InitializeAperiodicStreamOut(handle,
                                                          channel->getStreamOutIndex(),
                                                          channel->getTargetAddress(),
                                                          requestedScanRate),
                         "Cannot initialize aperiodic analog waveform output on LabJack LJM device") ||
                logError(LJM_WriteAperiodicStreamOut(handle,
                                                     channel->getStreamOutIndex(),
                                                     dataBuffer.size(),
                                                     dataBuffer.data(),
                                                     &bufferStatus),
                         "Cannot write aperiodic analog waveform output data to LabJack LJM device"))
            {
                return;
            }
        }
    }
    
    auto actualScanRate = requestedScanRate;
    if (logError(LJM_eStreamStart(handle,
                                  1,  // We aren't reading, so this is ignored (but still must be >0)
                                  addresses.size(),
                                  addresses.data(),
                                  &actualScanRate),
                 "Cannot start analog waveform output on LabJack LJM device"))
    {
        return;
    }
    if (actualScanRate != requestedScanRate) {
        mwarning(M_IODEVICE_MESSAGE_DOMAIN,
                 "LabJack LJM device cannot output analog waveforms at desired rate: "
                 "requested %g samples/second per channel, got %g samples/second per channel",
                 requestedScanRate,
                 actualScanRate);
    }
    
    analogWaveformsRunning = true;
}


void Device::stopAnalogWaveforms() {
    if (!analogWaveformsRunning) {
        return;
    }
    
    if (logError(LJM_eStreamStop(handle), "Cannot stop analog waveform output on LabJack LJM device") ||
        logError(analogWaveformsResetBuffer.write(handle), "Cannot reset analog waveform output on LabJack LJM device"))
    {
        return;
    }
    
    analogWaveformsRunning = false;
}


void Device::prepareDigitalInputs(WriteBuffer &configBuffer) {
    auto dioInhibit = ~std::uint32_t(0);
    
    for (auto &channel : digitalInputChannels) {
        channel->resolveLine(*deviceInfo);
        dioInhibit ^= channel->getDIOBitMask();
    }
    
    for (auto &channel : wordInputChannels) {
        channel->resolveLines(*deviceInfo);
        dioInhibit ^= channel->getDIOBitMask();
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
                             "Cannot set digital output line on LabJack LJM device");
                }
            }
        };
        channel->addNewValueNotification(boost::make_shared<VariableCallbackNotification>(callback));
    }
    
    for (auto &channel : wordOutputChannels) {
        channel->resolveLines(*deviceInfo);
        auto firstLineDIOIndex = channel->getFirstLineDIOIndex();
        auto dioBitMask = channel->getDIOBitMask();
        
        WriteBuffer updateBuffer;
        updateBuffer.append("DIO_INHIBIT", ~dioBitMask);
        updateBuffer.append("DIO_STATE");
        
        auto callback = [weakThis,
                         updateBuffer = std::move(updateBuffer),
                         dioBitMask,
                         firstLineDIOIndex](const Datum &data, MWTime time) mutable
        {
            if (auto sharedThis = weakThis.lock()) {
                lock_guard lock(sharedThis->mutex);
                if (sharedThis->running) {
                    updateBuffer.setValue(1, dioBitMask & (std::uint32_t(data.getInteger()) << firstLineDIOIndex));
                    logError(updateBuffer.write(sharedThis->handle),
                             "Cannot set word output lines on LabJack LJM device");
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
        auto dioBitMask = channel->getDIOBitMask();
        dioInhibit ^= dioBitMask;
        dioDirection |= dioBitMask;
        if (active) {
            dioState |= (channel->getValue() << channel->getDIOIndex());
        }
    }
    
    for (auto &channel : wordOutputChannels) {
        auto dioBitMask = channel->getDIOBitMask();
        dioInhibit ^= dioBitMask;
        dioDirection |= dioBitMask;
        if (active) {
            dioState |= dioBitMask & (std::uint32_t(channel->getValue()) << channel->getFirstLineDIOIndex());
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
        configBuffer.append(line + "_EF_INDEX", (channel->isHighSpeed() ? 7 : 8));
        
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
    
    if (logError(inputBuffer.read(handle), "Cannot read inputs from LabJack LJM device")) {
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
            channel->setValue(bool(dioState & channel->getDIOBitMask()), currentTime);
        }
        for (auto &channel : wordInputChannels) {
            channel->setValue((dioState & channel->getDIOBitMask()) >> channel->getFirstLineDIOIndex(), currentTime);
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
