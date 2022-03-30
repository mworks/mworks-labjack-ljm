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
    dioIndex(-1),
    dioBitMask(0)
{ }


void DigitalChannel::resolveLine(DeviceInfo &deviceInfo) {
    SingleLineChannel::resolveLine(deviceInfo);
    if (!(deviceInfo.isDIO(getLine()))) {
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN, boost::format("%s is not a digital line") % getLineName());
    }
    dioIndex = deviceInfo.getDIOIndex(getLine());
    dioBitMask = (1 << dioIndex);
}


void DigitalInputChannel::describeComponent(ComponentInfo &info) {
    DigitalChannel::describeComponent(info);
    info.setSignature("iochannel/labjack_ljm_digital_input");
}


void DigitalOutputChannel::describeComponent(ComponentInfo &info) {
    DigitalChannel::describeComponent(info);
    info.setSignature("iochannel/labjack_ljm_digital_output");
}


const std::string WordChannel::NUM_LINES("num_lines");


void WordChannel::describeComponent(ComponentInfo &info) {
    MultipleLineChannel::describeComponent(info);
    info.addParameter(NUM_LINES);
}


WordChannel::WordChannel(const ParameterValueMap &parameters) :
    MultipleLineChannel(parameters),
    numLines(parameters[NUM_LINES]),
    firstLineDIOIndex(-1),
    dioBitMask(0)
{
    if (numLines < 1) {
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN, "Word channel must use at least one line");
    }
}


void WordChannel::resolveLines(DeviceInfo &deviceInfo) {
    MultipleLineChannel::resolveLines(deviceInfo);
    
    if (!(deviceInfo.isDIO(getFirstLine()))) {
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN,
                              boost::format("%s is not a digital line") % getFirstLineName());
    }
    if (!(deviceInfo.isDIO(getFirstLine() + numLines - 1))) {
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN,
                              boost::format("Word channel starting at line %s requires more digital lines than are "
                                            "available on the device")
                              % getFirstLineName());
    }
    
    for (int i = 1; i < numLines; i++) {
        if (!deviceInfo.reserveLine(getFirstLine() + i)) {
            throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN,
                                  boost::format("Word channel starting at line %s requires lines that are already "
                                                "in use")
                                  % getFirstLineName());
        }
    }
    
    firstLineDIOIndex = deviceInfo.getDIOIndex(getFirstLine());
    for (int i = 0; i < numLines; i++) {
        dioBitMask |= (1 << (firstLineDIOIndex + i));
    }
}


void WordInputChannel::describeComponent(ComponentInfo &info) {
    WordChannel::describeComponent(info);
    info.setSignature("iochannel/labjack_ljm_word_input");
}


void WordOutputChannel::describeComponent(ComponentInfo &info) {
    WordChannel::describeComponent(info);
    info.setSignature("iochannel/labjack_ljm_word_output");
}


END_NAMESPACE_MW_LABJACK_LJM
