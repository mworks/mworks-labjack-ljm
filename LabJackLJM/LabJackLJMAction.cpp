//
//  LabJackLJMAction.cpp
//  LabJackLJM
//
//  Created by Christopher Stawarz on 2/16/24.
//  Copyright © 2024 The MWorks Project. All rights reserved.
//

#include "LabJackLJMAction.hpp"


BEGIN_NAMESPACE_MW_LABJACK_LJM


const std::string Action::DEVICE("device");


void Action::describeComponent(ComponentInfo &info) {
    mw::Action::describeComponent(info);
    info.addParameter(DEVICE);
}


Action::Action(const ParameterValueMap &parameters) :
    mw::Action(parameters)
{
    auto &deviceParam = parameters[DEVICE];
    auto &deviceName = deviceParam.str();
    auto device = deviceParam.getRegistry()->getObject<Device>(deviceName);
    if (!device) {
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN, "Device is not a LabJack LJM device", deviceName);
    }
    weakDevice = device;
}


void StartAnalogWaveformOutputAction::describeComponent(ComponentInfo &info) {
    Action::describeComponent(info);
    info.setSignature("action/labjack_ljm_start_analog_waveform_output");
}


bool StartAnalogWaveformOutputAction::execute() {
    if (auto device = getDevice()) {
        device->startAnalogWaveformOutput();
    }
    return true;
}


void StopAnalogWaveformOutputAction::describeComponent(ComponentInfo &info) {
    Action::describeComponent(info);
    info.setSignature("action/labjack_ljm_stop_analog_waveform_output");
}


bool StopAnalogWaveformOutputAction::execute() {
    if (auto device = getDevice()) {
        device->stopAnalogWaveformOutput();
    }
    return true;
}


END_NAMESPACE_MW_LABJACK_LJM
