//
//  LabJackLJMCounterChannel.hpp
//  LabJackLJM
//
//  Created by Christopher Stawarz on 3/24/22.
//  Copyright © 2022 The MWorks Project. All rights reserved.
//

#ifndef LabJackLJMCounterChannel_hpp
#define LabJackLJMCounterChannel_hpp

#include "LabJackLJMChannel.hpp"


BEGIN_NAMESPACE_MW_LABJACK_LJM


class CounterChannel : public SingleLineChannel {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    explicit CounterChannel(const ParameterValueMap &parameters);
    
    void resolveLine(DeviceInfo &deviceInfo) override;
    
    bool isHighSpeed() const { return highSpeed; }
    
    void setValue(long long value, MWTime time) const {
        if (getValueVar()->getValue().getInteger() != value) {
            getValueVar()->setValue(Datum(value), time);
        }
    }
    
private:
    bool highSpeed;
    
};


END_NAMESPACE_MW_LABJACK_LJM


#endif /* LabJackLJMCounterChannel_hpp */
