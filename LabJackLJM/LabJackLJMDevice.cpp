//
//  LabJackLJMDevice.cpp
//  LabJackLJM
//
//  Created by Christopher Stawarz on 11/20/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#include "LabJackLJMDevice.hpp"


BEGIN_NAMESPACE_MW_LABJACK_LJM


template<>
inline std::uint32_t Device::Stream::DataReader::get() {
    return std::uint32_t(next()) | (std::uint32_t(next()) << 16);
}


const std::string Device::DEVICE_TYPE("device_type");
const std::string Device::CONNECTION_TYPE("connection_type");
const std::string Device::IDENTIFIER("identifier");
const std::string Device::DATA_INTERVAL("data_interval");
const std::string Device::UPDATE_INTERVAL("update_interval");
const std::string Device::CLOCK_OFFSET_NANOS("clock_offset_nanos");


void Device::describeComponent(ComponentInfo &info) {
    IODevice::describeComponent(info);
    
    info.setSignature("iodevice/labjack_ljm");
    
    info.addParameter(DEVICE_TYPE, "ANY");
    info.addParameter(CONNECTION_TYPE, "ANY");
    info.addParameter(IDENTIFIER, "ANY");
    info.addParameter(DATA_INTERVAL);
    info.addParameter(UPDATE_INTERVAL);
    info.addParameter(CLOCK_OFFSET_NANOS, false);
}


Device::Device(const ParameterValueMap &parameters) :
    IODevice(parameters),
    deviceType(variableOrText(parameters[DEVICE_TYPE])->getValue().getString()),
    connectionType(variableOrText(parameters[CONNECTION_TYPE])->getValue().getString()),
    identifier(variableOrText(parameters[IDENTIFIER])->getValue().getString()),
    dataInterval(parameters[DATA_INTERVAL]),
    updateInterval(parameters[UPDATE_INTERVAL]),
    clockOffsetNanosVar(optionalVariable(parameters[CLOCK_OFFSET_NANOS])),
    clock(Clock::instance()),
    handle(-1),
    writeBuffer(handle),
    stream(*this),
    currentClockOffsetNanos(0),
    coreTimerNanosAtLastClockSync(0),
    lastClockSyncUpdateTime(0),
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
        writeBuffer.append("LED_COMM", 0);
        logError(writeBuffer.write(), "Cannot turn off LJM device COMM LED");
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
    
    writeBuffer.append("IO_CONFIG_SET_CURRENT_TO_FACTORY", 1);
    writeBuffer.append("POWER_LED", 4);  // Set LED operation to manual
    writeBuffer.append("LED_STATUS", 1);
    writeBuffer.append("LED_COMM", 1);
    
    if (haveInputs()) {
        stream.add("CORE_TIMER");
        stream.add("STREAM_DATA_CAPTURE_16");  // CORE_TIMER is a 32-bit value
    }
    if (haveDigitalInputs()) {
        prepareDigitalInputs();
    }
    if (haveDigitalOutputs()) {
        prepareDigitalOutputs();
    }
    
    if (logError(writeBuffer.write(), "Cannot configure LJM device")) {
        return false;
    }
    
    return true;
}


bool Device::startDeviceIO() {
    lock_guard lock(mutex);
    
    if (!running) {
        if (haveDigitalOutputs()) {
            updateDigitalOutputs(true);
        }
        
        if (logError(writeBuffer.write(), "Cannot start LJM device")) {
            return false;
        }
        
        const auto currentTime = clock->getCurrentTimeUS();
        
        if (haveInputs()) {
            updateClockSync(currentTime);
            if (!stream.start()) {
                return false;
            }
        }
        
        running = true;
    }
    
    return true;
}


bool Device::stopDeviceIO() {
    lock_guard lock(mutex);
    
    if (running) {
        if (haveInputs()) {
            if (!stream.stop()) {
                return false;
            }
        }
        
        if (haveDigitalOutputs()) {
            updateDigitalOutputs();
        }
        
        if (logError(writeBuffer.write(), "Cannot stop LJM device")) {
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


void Device::prepareDigitalInputs() {
    auto dioInhibit = ~std::uint32_t(0);
    
    for (auto &channel : digitalInputChannels) {
        validateDigitalChannel(channel);
        dioInhibit ^= (1 << channel->getDIOIndex());
    }
    
    writeBuffer.append("DIO_INHIBIT", dioInhibit);
    writeBuffer.append("DIO_DIRECTION", 0);
    
    stream.add("FIO_EIO_STATE");
    stream.add("CIO_MIO_STATE");
}


void Device::prepareDigitalOutputs() {
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
    
    updateDigitalOutputs();
}


void Device::updateDigitalOutputs(bool active) {
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
    writeBuffer.append("DIO_DIRECTION", dioDirection);
    writeBuffer.append("DIO_STATE", dioState);
}


void Device::readInputs() {
    if (!stream.read()) {
        return;
    }
    
    const auto currentTime = clock->getCurrentTimeUS();
    auto data = stream.getData();
    
    do {
        const auto coreTimer = data.get<CoreTimerValue>();
        const auto scanTime = applyClockOffset(coreTimer, currentTime);
        if (haveDigitalInputs()) {
            const auto dioState = data.get<std::uint32_t>();
            for (auto &channel : digitalInputChannels) {
                channel->setValue(bool(dioState & (1 << channel->getDIOIndex())), scanTime);
            }
        }
    } while (data);
    
    if (currentTime - lastClockSyncUpdateTime >= clockSyncUpdateInterval) {
        updateClockSync(currentTime);
    }
}


void Device::updateClockSync(MWTime currentTime) {
    // Reset stored clock offset to zero, so that we know not to use it if the
    // update fails
    currentClockOffsetNanos = 0;
    
    std::array<std::tuple<MWTime, MWTime, CoreTimerValue>, 5> samples;
    int type;
    auto address = convertNameToAddress("CORE_TIMER", type);
    double value;
    
    for (auto &sample : samples) {
        auto beforeNS = clock->getCurrentTimeNS();
        auto result = LJM_eReadAddress(handle, address, type, &value);
        auto afterNS = clock->getCurrentTimeNS();
        if (logError(result, "Cannot read LJM device system timer")) {
            return;
        }
        sample = std::make_tuple(beforeNS, afterNS - beforeNS, CoreTimerValue(value));
    }
    
    // Sort by afterNS-beforeNS and extract median (to exclude outliers)
    std::sort(samples.begin(), samples.end(), [](const auto &first, const auto &second) {
        return std::get<1>(first) < std::get<1>(second);
    });
    const auto &median = samples.at(samples.size() / 2);
    
    coreTimerNanosAtLastClockSync = coreTimerTicksToNanos(std::get<2>(median));
    currentClockOffsetNanos = (std::get<0>(median) + std::get<1>(median) / 2) - coreTimerNanosAtLastClockSync;
    if (clockOffsetNanosVar) {
        clockOffsetNanosVar->setValue(Datum(currentClockOffsetNanos));
    }
    
    lastClockSyncUpdateTime = currentTime;
}


MWTime Device::applyClockOffset(CoreTimerValue coreTimer, MWTime currentTime) const {
    if (0 == currentClockOffsetNanos) {
        return currentTime;
    }
    auto coreTimerNanos = coreTimerTicksToNanos(coreTimer);
    if (coreTimerNanos < coreTimerNanosAtLastClockSync) {
        // Core timer wrapped
        coreTimerNanos += coreTimerTicksToNanos(std::numeric_limits<CoreTimerValue>::max());
    }
    return (coreTimerNanos + currentClockOffsetNanos) / 1000;  // ns to us
}


void Device::WriteBuffer::append(const std::string &name, double value) {
    int type;
    auto address = convertNameToAddress(name, type);
    names.emplace_back(name);
    addresses.push_back(address);
    types.push_back(type);
    values.push_back(value);
}


int Device::WriteBuffer::write() {
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
    
    names.clear();
    addresses.clear();
    types.clear();
    values.clear();
    
    return result;
}


void Device::Stream::add(const std::string &name) {
    addresses.push_back(convertNameToAddress(name));
}


bool Device::Stream::start() {
    scansPerRead = device.updateInterval / device.dataInterval;
    const auto numAddresses = addresses.size();
    data.resize(numAddresses * scansPerRead);
    
    const auto desiredScanRate = 1e6 * double(scansPerRead) / double(device.updateInterval);  // scans per second
    auto actualScanRate = desiredScanRate;
    
    if (logError(LJM_eStreamStart(device.handle, scansPerRead, numAddresses, addresses.data(), &actualScanRate),
                 "Cannot start LJM input stream") ||
        logError(LJM_SetStreamCallback(device.handle, callback, &device),
                 "Cannot set LJM input stream callback"))
    {
        return false;
    }
    
    if (actualScanRate != desiredScanRate) {
        mwarning(M_IODEVICE_MESSAGE_DOMAIN,
                 "LJM device cannot scan inputs at desired rate: "
                 "requested %g scans/second, got %g scans/second",
                 desiredScanRate,
                 actualScanRate);
    }
    
    return true;
}


bool Device::Stream::read() {
    int deviceScanBacklog = 0;
    int ljmScanBacklog = 0;
    
    if (logError(LJM_eStreamRead(device.handle, data.data(), &deviceScanBacklog, &ljmScanBacklog),
                 "LJM stream read failed"))
    {
        return false;
    }
    
    // Warn if there's more than one read's worth of scans in either buffer's backlog
    if (deviceScanBacklog > scansPerRead) {
        mwarning(M_IODEVICE_MESSAGE_DOMAIN, "LJM device buffer contains %d unread input scans", deviceScanBacklog);
    }
    if (ljmScanBacklog > scansPerRead) {
        mwarning(M_IODEVICE_MESSAGE_DOMAIN, "LJM library buffer contains %d unread input scans", ljmScanBacklog);
    }
    
    return true;
}


bool Device::Stream::stop() {
    if (logError(LJM_eStreamStop(device.handle), "Cannot stop LJM input stream")) {
        return false;
    }
    return true;
}


void Device::Stream::callback(void *_device) {
    auto &device = *static_cast<Device *>(_device);
    lock_guard lock(device.mutex);
    device.readInputs();
}


END_NAMESPACE_MW_LABJACK_LJM
