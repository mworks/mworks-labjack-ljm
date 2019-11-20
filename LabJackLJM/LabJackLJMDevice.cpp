//
//  LabJackLJMDevice.cpp
//  LabJackLJM
//
//  Created by Christopher Stawarz on 11/20/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#include "LabJackLJMDevice.hpp"


BEGIN_NAMESPACE_MW


BEGIN_NAMESPACE()


inline std::string formatLJMError(int error, const char *description) {
    std::array<char, LJM_MAX_NAME_SIZE> msg;
    LJM_ErrorToString(error, msg.data());
    return (boost::format("%s; error = %d (%s)") % description % error % msg.data()).str();
}


inline bool logLJMError(int error, const char *description) {
    const bool failed = (LJME_NOERROR != error);
    if (failed) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "%s", formatLJMError(error, description).c_str());
    }
    return failed;
}


inline void throwLJMError(int error, const char *description) {
    if (LJME_NOERROR != error) {
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN, formatLJMError(error, description));
    }
}


END_NAMESPACE()


const std::string LabJackLJMDevice::DEVICE_TYPE("device_type");
const std::string LabJackLJMDevice::CONNECTION_TYPE("connection_type");
const std::string LabJackLJMDevice::IDENTIFIER("identifier");


void LabJackLJMDevice::describeComponent(ComponentInfo &info) {
    IODevice::describeComponent(info);
    
    info.setSignature("iodevice/labjack_ljm");
    
    info.addParameter(DEVICE_TYPE, "ANY");
    info.addParameter(CONNECTION_TYPE, "ANY");
    info.addParameter(IDENTIFIER, "ANY");
}


LabJackLJMDevice::LabJackLJMDevice(const ParameterValueMap &parameters) :
    IODevice(parameters),
    deviceType(variableOrText(parameters[DEVICE_TYPE])->getValue().getString()),
    connectionType(variableOrText(parameters[CONNECTION_TYPE])->getValue().getString()),
    identifier(variableOrText(parameters[IDENTIFIER])->getValue().getString()),
    handle(-1)
{
}


LabJackLJMDevice::~LabJackLJMDevice() {
    if (-1 != handle) {
        (void)logLJMError(LJM_Close(handle),
                          "Cannot close LJM device");
    }
}


bool LabJackLJMDevice::initialize() {
    if (logLJMError(LJM_OpenS(deviceType.c_str(), connectionType.c_str(), identifier.c_str(), &handle),
                    "Cannot open LJM device") ||
        logLJMError(LJM_eWriteName(handle, "IO_CONFIG_SET_CURRENT_TO_FACTORY", 1),
                    "Cannot reset LJM device to factory configuration"))
    {
        return false;
    }
    
    return true;
}


END_NAMESPACE_MW
