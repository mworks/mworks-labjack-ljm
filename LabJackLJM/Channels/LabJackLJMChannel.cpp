//
//  LabJackLJMChannel.cpp
//  LabJackLJM
//
//  Created by Christopher Stawarz on 11/22/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#include "LabJackLJMChannel.hpp"


BEGIN_NAMESPACE_MW_LABJACK_LJM


Channel::Channel(const ParameterValueMap &parameters) :
    Component(parameters)
{ }


const std::string SingleLineChannel::LINE("line");
const std::string SingleLineChannel::VALUE("value");


void SingleLineChannel::describeComponent(ComponentInfo &info) {
    Channel::describeComponent(info);
    info.addParameter(LINE);
    info.addParameter(VALUE);
}


SingleLineChannel::SingleLineChannel(const ParameterValueMap &parameters) :
    Channel(parameters),
    lineName(variableOrText(parameters[LINE])->getValue().getString()),
    value(parameters[VALUE])
{ }


END_NAMESPACE_MW_LABJACK_LJM
