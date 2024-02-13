//
//  LabJackLJMAnalogChannel.hpp
//  LabJackLJM
//
//  Created by Christopher Stawarz on 3/17/22.
//  Copyright © 2022 The MWorks Project. All rights reserved.
//

#ifndef LabJackLJMAnalogChannel_hpp
#define LabJackLJMAnalogChannel_hpp

#include "LabJackLJMChannel.hpp"


BEGIN_NAMESPACE_MW_LABJACK_LJM


class AnalogChannel : public SingleLineChannel {
    
public:
    using SingleLineChannel::SingleLineChannel;
    
};


class AnalogInputChannel : public AnalogChannel {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    using AnalogChannel::AnalogChannel;
    
    void resolveLine(DeviceInfo &deviceInfo) override;
    
    void setValue(double value, MWTime time) const {
        // Analog signals are expected to be continuous, so update the variable
        // even if the value hasn't changed
        getValueVar()->setValue(Datum(value), time);
    }
    
};


class AnalogOutputChannel : public AnalogChannel {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    using AnalogChannel::AnalogChannel;
    
    void resolveLine(DeviceInfo &deviceInfo) override;
    
    void addNewValueNotification(const boost::shared_ptr<VariableNotification> &notification) const {
        getValueVar()->addNotification(notification);
    }
    
    double getValue() const {
        return getValueVar()->getValue().getFloat();
    }
    
};


class AnalogWaveformChannel : public AnalogChannel {
    
public:
    static const std::string LOOP;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit AnalogWaveformChannel(const ParameterValueMap &parameters);
    
    void resolveLine(DeviceInfo &deviceInfo) override;
    
    void setStreamOutIndex(int index) { streamOutIndex = index; }
    int getStreamOutIndex() const { return streamOutIndex; }
    
    void setStreamOutAddress(int address) { streamOutAddress = address; }
    int getStreamOutAddress() const { return streamOutAddress; }
    
    void setTargetAddress(int address) { targetAddress = address; }
    int getTargetAddress() const { return targetAddress; }
    
    bool getData(std::vector<double> &dataBuffer) const;
    bool getLoop() const { return loop->getValue().getBool(); }
    
private:
    const VariablePtr loop;
    int streamOutIndex;
    int streamOutAddress;
    int targetAddress;
    
};


END_NAMESPACE_MW_LABJACK_LJM


#endif /* LabJackLJMAnalogChannel_hpp */
