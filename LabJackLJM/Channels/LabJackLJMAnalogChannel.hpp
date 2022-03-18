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
    
    int resolveLine(const DeviceInfo &deviceInfo) override;
    
    void setValue(double value, MWTime time) const {
        // Analog signals are expected to be continuous, so update the variable
        // even if the value hasn't changed
        valueVar->setValue(Datum(value), time);
    }
    
};


class AnalogOutputChannel : public AnalogChannel {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    using AnalogChannel::AnalogChannel;
    
    int resolveLine(const DeviceInfo &deviceInfo) override;
    
    void addNewValueNotification(const boost::shared_ptr<VariableNotification> &notification) const {
        valueVar->addNotification(notification);
    }
    
    double getValue() const {
        return valueVar->getValue().getFloat();
    }
    
};


END_NAMESPACE_MW_LABJACK_LJM


#endif /* LabJackLJMAnalogChannel_hpp */
