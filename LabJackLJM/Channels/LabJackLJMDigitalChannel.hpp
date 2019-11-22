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
    
    int getDIOIndex() const {
        return (static_cast<int>(getLine()) - static_cast<int>(PhysicalLine::DIO_MIN));
    }
    
};


class DigitalInputChannel : public DigitalChannel {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    using DigitalChannel::DigitalChannel;
    
};


class DigitalOutputChannel : public DigitalChannel {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    using DigitalChannel::DigitalChannel;
    
};


END_NAMESPACE_MW_LABJACK_LJM


#endif /* LabJackLJMDigitalChannel_hpp */
