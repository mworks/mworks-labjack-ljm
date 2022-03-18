//
//  LabJackLJMDigitalChannel.cpp
//  LabJackLJM
//
//  Created by Christopher Stawarz on 12/6/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#include "LabJackLJMDigitalChannel.hpp"


BEGIN_NAMESPACE_MW_LABJACK_LJM


DigitalChannel::DigitalChannel(const ParameterValueMap &parameters) :
    SingleLineChannel(parameters),
    dioIndex(-1)
{ }


int DigitalChannel::resolveLine(const DeviceInfo &deviceInfo) {
    auto line = SingleLineChannel::resolveLine(deviceInfo);
    if (!(deviceInfo.isDIO(line))) {
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN, boost::format("%s is not a digital line") % lineName);
    }
    dioIndex = deviceInfo.getDIOIndex(line);
    canonicalLineName = (boost::format("DIO%d") % dioIndex).str();
    return line;
}


void DigitalInputChannel::describeComponent(ComponentInfo &info) {
    DigitalChannel::describeComponent(info);
    info.setSignature("iochannel/labjack_ljm_digital_input");
}


void DigitalOutputChannel::describeComponent(ComponentInfo &info) {
    DigitalChannel::describeComponent(info);
    info.setSignature("iochannel/labjack_ljm_digital_output");
}


void CounterChannel::describeComponent(ComponentInfo &info) {
    DigitalChannel::describeComponent(info);
    info.setSignature("iochannel/labjack_ljm_counter");
}


int CounterChannel::resolveLine(const DeviceInfo &deviceInfo) {
    auto line = DigitalChannel::resolveLine(deviceInfo);
    if (!(deviceInfo.isHighSpeedCounter(line))) {
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN,
                              boost::format("%s is not a high-speed counter input line") % lineName);
    }
    return line;
}


END_NAMESPACE_MW_LABJACK_LJM
