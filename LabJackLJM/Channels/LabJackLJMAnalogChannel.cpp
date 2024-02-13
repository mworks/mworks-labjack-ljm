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
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN,
                              boost::format("%s is not an analog input line") % getLineName());
    }
}


void AnalogOutputChannel::describeComponent(ComponentInfo &info) {
    AnalogChannel::describeComponent(info);
    info.setSignature("iochannel/labjack_ljm_analog_output");
}


void AnalogOutputChannel::resolveLine(DeviceInfo &deviceInfo) {
    AnalogChannel::resolveLine(deviceInfo);
    if (!(deviceInfo.isDAC(getLine()))) {
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN,
                              boost::format("%s is not an analog output line") % getLineName());
    }
}


const std::string AnalogWaveformChannel::LOOP("loop");


void AnalogWaveformChannel::describeComponent(ComponentInfo &info) {
    AnalogChannel::describeComponent(info);
    
    info.setSignature("iochannel/labjack_ljm_analog_waveform");
    
    info.addParameterAlias(VALUE, "data");
    info.addParameter(LOOP, "NO");
}


AnalogWaveformChannel::AnalogWaveformChannel(const ParameterValueMap &parameters) :
    AnalogChannel(parameters),
    loop(parameters[LOOP]),
    streamOutIndex(-1),
    streamOutAddress(-1),
    targetAddress(-1)
{ }


void AnalogWaveformChannel::resolveLine(DeviceInfo &deviceInfo) {
    AnalogChannel::resolveLine(deviceInfo);
    if (!(deviceInfo.isDAC(getLine()))) {
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN,
                              boost::format("%s is not an analog output line") % getLineName());
    }
}


bool AnalogWaveformChannel::getData(std::vector<double> &dataBuffer) const {
    const auto data = getValueVar()->getValue();
    
    switch (data.getDataType()) {
        case M_LIST: {
            auto &list = data.getList();
            dataBuffer.reserve(list.size());
            for (auto &item : list) {
                if (!item.isNumber()) {
                    merror(M_IODEVICE_MESSAGE_DOMAIN,
                           "LabJack LJM analog waveform data list contains a non-numeric value");
                    return false;
                }
                dataBuffer.push_back(item.getFloat());
            }
            break;
        }
            
        case M_STRING: {
            auto &string = data.getString();
            constexpr auto valueSize = sizeof(double);
            if (string.size() % valueSize != 0) {
                merror(M_IODEVICE_MESSAGE_DOMAIN,
                       "LabJack LJM analog waveform data string contains an invalid number of bytes");
                return false;
            }
            auto data = reinterpret_cast<const double *>(string.data());
            dataBuffer.assign(data, data + (string.size() / valueSize));
            break;
        }
            
        default: {
            merror(M_IODEVICE_MESSAGE_DOMAIN,
                   "LabJack LJM analog waveform data must be provided as a list or a string");
            return false;
        }
    }
    
    return true;
}


END_NAMESPACE_MW_LABJACK_LJM
