//
//  LabJackLJMAnalogChannel.cpp
//  LabJackLJM
//
//  Created by Christopher Stawarz on 3/17/22.
//  Copyright © 2022 The MWorks Project. All rights reserved.
//

#include "LabJackLJMAnalogChannel.hpp"


BEGIN_NAMESPACE_MW_LABJACK_LJM


void AnalogInputChannel::describeComponent(ComponentInfo &info) {
    AnalogChannel::describeComponent(info);
    info.setSignature("iochannel/labjack_ljm_analog_input");
}


void AnalogInputChannel::resolveLine(DeviceInfo &deviceInfo) {
    AnalogChannel::resolveLine(deviceInfo);
    if (!(deviceInfo.isAIN(getLine()))) {
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN, boost::format("%s is not an analog input line") % lineName);
    }
}


void AnalogOutputChannel::describeComponent(ComponentInfo &info) {
    AnalogChannel::describeComponent(info);
    info.setSignature("iochannel/labjack_ljm_analog_output");
}


void AnalogOutputChannel::resolveLine(DeviceInfo &deviceInfo) {
    AnalogChannel::resolveLine(deviceInfo);
    if (!(deviceInfo.isDAC(getLine()))) {
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN, boost::format("%s is not an analog output line") % lineName);
    }
}


END_NAMESPACE_MW_LABJACK_LJM
