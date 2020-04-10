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


void DigitalInputChannel::describeComponent(ComponentInfo &info) {
    DigitalChannel::describeComponent(info);
    info.setSignature("iochannel/labjack_ljm_digital_input");
}


void DigitalOutputChannel::describeComponent(ComponentInfo &info) {
    DigitalChannel::describeComponent(info);
    info.setSignature("iochannel/labjack_ljm_digital_output");
}


END_NAMESPACE_MW_LABJACK_LJM
