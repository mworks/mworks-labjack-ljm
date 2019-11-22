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


auto Channel::parseLineName(const std::string &name) -> PhysicalLine {
    static const std::map<std::string, PhysicalLine> validNames {
        { "AIN0", PhysicalLine::AIN0 },
        { "AIN1", PhysicalLine::AIN1 },
        { "AIN2", PhysicalLine::AIN2 },
        { "AIN3", PhysicalLine::AIN3 },
        { "AIN4", PhysicalLine::AIN4 },
        { "AIN5", PhysicalLine::AIN5 },
        { "AIN6", PhysicalLine::AIN6 },
        { "AIN7", PhysicalLine::AIN7 },
        { "AIN8", PhysicalLine::AIN8 },
        { "AIN9", PhysicalLine::AIN9 },
        { "AIN10", PhysicalLine::AIN10 },
        { "AIN11", PhysicalLine::AIN11 },
        { "AIN12", PhysicalLine::AIN12 },
        { "AIN13", PhysicalLine::AIN13 },
        { "DAC0", PhysicalLine::DAC0 },
        { "DAC1", PhysicalLine::DAC1 },
        { "FIO0", PhysicalLine::FIO0 },
        { "FIO1", PhysicalLine::FIO1 },
        { "FIO2", PhysicalLine::FIO2 },
        { "FIO3", PhysicalLine::FIO3 },
        { "FIO4", PhysicalLine::FIO4 },
        { "FIO5", PhysicalLine::FIO5 },
        { "FIO6", PhysicalLine::FIO6 },
        { "FIO7", PhysicalLine::FIO7 },
        { "EIO0", PhysicalLine::EIO0 },
        { "EIO1", PhysicalLine::EIO1 },
        { "EIO2", PhysicalLine::EIO2 },
        { "EIO3", PhysicalLine::EIO3 },
        { "EIO4", PhysicalLine::EIO4 },
        { "EIO5", PhysicalLine::EIO5 },
        { "EIO6", PhysicalLine::EIO6 },
        { "EIO7", PhysicalLine::EIO7 },
        { "CIO0", PhysicalLine::CIO0 },
        { "CIO1", PhysicalLine::CIO1 },
        { "CIO2", PhysicalLine::CIO2 },
        { "CIO3", PhysicalLine::CIO3 },
        { "MIO0", PhysicalLine::MIO0 },
        { "MIO1", PhysicalLine::MIO1 },
        { "MIO2", PhysicalLine::MIO2 },
        { "DIO0", PhysicalLine::DIO0 },
        { "DIO1", PhysicalLine::DIO1 },
        { "DIO2", PhysicalLine::DIO2 },
        { "DIO3", PhysicalLine::DIO3 },
        { "DIO4", PhysicalLine::DIO4 },
        { "DIO5", PhysicalLine::DIO5 },
        { "DIO6", PhysicalLine::DIO6 },
        { "DIO7", PhysicalLine::DIO7 },
        { "DIO8", PhysicalLine::DIO8 },
        { "DIO9", PhysicalLine::DIO9 },
        { "DIO10", PhysicalLine::DIO10 },
        { "DIO11", PhysicalLine::DIO11 },
        { "DIO12", PhysicalLine::DIO12 },
        { "DIO13", PhysicalLine::DIO13 },
        { "DIO14", PhysicalLine::DIO14 },
        { "DIO15", PhysicalLine::DIO15 },
        { "DIO16", PhysicalLine::DIO16 },
        { "DIO17", PhysicalLine::DIO17 },
        { "DIO18", PhysicalLine::DIO18 },
        { "DIO19", PhysicalLine::DIO19 },
        { "DIO20", PhysicalLine::DIO20 },
        { "DIO21", PhysicalLine::DIO21 },
        { "DIO22", PhysicalLine::DIO22 },
    };
    
    auto iter = validNames.find(boost::algorithm::to_upper_copy(name));
    if (iter != validNames.end()) {
        return iter->second;
    }
    
    throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN, boost::format("Invalid line name: \"%s\"") % name);
}


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
    line(parseLineName(lineName)),
    value(parameters[VALUE])
{ }


END_NAMESPACE_MW_LABJACK_LJM
