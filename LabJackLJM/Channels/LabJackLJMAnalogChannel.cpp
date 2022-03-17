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


AnalogInputChannel::AnalogInputChannel(const ParameterValueMap &parameters) :
    AnalogChannel(parameters),
    flexibleIO(false),
    dioIndex(-1)
{ }


int AnalogInputChannel::resolveLine(const DeviceInfo &deviceInfo) {
    auto line = AnalogChannel::resolveLine(deviceInfo);
    if (!(deviceInfo.isAIN(line))) {
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN, boost::format("%s is not an analog input line") % lineName);
    }
    canonicalLineName = (boost::format("AIN%d") % deviceInfo.getAINIndex(line)).str();
    flexibleIO = deviceInfo.isFlexibleIO(line);
    if (flexibleIO) {
        dioIndex = deviceInfo.getDIOIndex(line);
    }
    return line;
}


void AnalogOutputChannel::describeComponent(ComponentInfo &info) {
    AnalogChannel::describeComponent(info);
    info.setSignature("iochannel/labjack_ljm_analog_output");
}


int AnalogOutputChannel::resolveLine(const DeviceInfo &deviceInfo) {
    auto line = AnalogChannel::resolveLine(deviceInfo);
    if (!(deviceInfo.isDAC(line))) {
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN, boost::format("%s is not an analog output line") % lineName);
    }
    canonicalLineName = (boost::format("DAC%d") % deviceInfo.getDACIndex(line)).str();
    return line;
}


END_NAMESPACE_MW_LABJACK_LJM
