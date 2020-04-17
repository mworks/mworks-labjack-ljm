//
//  LabJackLJMDigitalChannel.hpp
//  LabJackLJM
//
//  Created by Christopher Stawarz on 12/6/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#ifndef LabJackLJMDigitalChannel_hpp
#define LabJackLJMDigitalChannel_hpp

#include "LabJackLJMChannel.hpp"


BEGIN_NAMESPACE_MW_LABJACK_LJM


class DigitalChannel : public SingleLineChannel {
    
public:
    explicit DigitalChannel(const ParameterValueMap &parameters);
    
    void setDIOIndex(int index) { dioIndex = index; }
    int getDIOIndex() const { return dioIndex; }
    
private:
    int dioIndex;
    
};


class DigitalInputChannel : public DigitalChannel {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    using DigitalChannel::DigitalChannel;
    
    void setValue(bool value, MWTime time) const;
    
};


class DigitalOutputChannel : public DigitalChannel {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    using DigitalChannel::DigitalChannel;
    
    void addNewValueNotification(const boost::shared_ptr<VariableNotification> &notification) const {
        valueVar->addNotification(notification);
    }
    
    bool getValue() const;
    
};


END_NAMESPACE_MW_LABJACK_LJM


#endif /* LabJackLJMDigitalChannel_hpp */
