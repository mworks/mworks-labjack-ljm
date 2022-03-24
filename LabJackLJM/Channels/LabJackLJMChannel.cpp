//
//  LabJackLJMChannel.cpp
//  LabJackLJM
//
//  Created by Christopher Stawarz on 11/22/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#include "LabJackLJMChannel.hpp"


BEGIN_NAMESPACE_MW_LABJACK_LJM


const std::string Channel::VALUE("value");


void Channel::describeComponent(ComponentInfo &info) {
    Component::describeComponent(info);
    info.addParameter(VALUE);
}


Channel::Channel(const ParameterValueMap &parameters) :
    Component(parameters),
    valueVar(parameters[VALUE])
{ }


const std::string SingleLineChannel::LINE("line");


void SingleLineChannel::describeComponent(ComponentInfo &info) {
    Channel::describeComponent(info);
    info.addParameter(LINE);
}


SingleLineChannel::SingleLineChannel(const ParameterValueMap &parameters) :
    Channel(parameters),
    lineName(variableOrText(parameters[LINE])->getValue().getString()),
    line(-1)
{ }


void SingleLineChannel::resolveLine(DeviceInfo &deviceInfo) {
    line = deviceInfo.reserveLineForName(lineName);
    canonicalLineName = deviceInfo.getCanonicalLineName(line);
}


const std::string MultipleLineChannel::FIRST_LINE("first_line");


void MultipleLineChannel::describeComponent(ComponentInfo &info) {
    Channel::describeComponent(info);
    info.addParameter(FIRST_LINE);
}


MultipleLineChannel::MultipleLineChannel(const ParameterValueMap &parameters) :
    Channel(parameters),
    firstLineName(variableOrText(parameters[FIRST_LINE])->getValue().getString())
{ }


void MultipleLineChannel::resolveLines(DeviceInfo &deviceInfo) {
    firstLine = deviceInfo.reserveLineForName(firstLineName);
}


END_NAMESPACE_MW_LABJACK_LJM
