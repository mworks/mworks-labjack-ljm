//
//  LabJackLJMQuadratureChannel.hpp
//  LabJackLJM
//
//  Created by Christopher Stawarz on 3/28/22.
//  Copyright © 2022 The MWorks Project. All rights reserved.
//

#ifndef LabJackLJMQuadratureChannel_hpp
#define LabJackLJMQuadratureChannel_hpp

#include "LabJackLJMChannel.hpp"


BEGIN_NAMESPACE_MW_LABJACK_LJM


class QuadratureInputChannel : public MultipleLineChannel {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    explicit QuadratureInputChannel(const ParameterValueMap &parameters);
    
    void resolveLines(DeviceInfo &deviceInfo) override;
    
    const std::string & getCanonicalFirstLineName() const { return canonicalFirstLineName; }
    const std::string & getCanonicalSecondLineName() const { return canonicalSecondLineName; }
    
    void setValue(long long value, MWTime time) const {
        if (getValueVar()->getValue().getInteger() != value) {
            getValueVar()->setValue(Datum(value), time);
        }
    }
    
private:
    std::string canonicalFirstLineName;
    std::string canonicalSecondLineName;
    
};


END_NAMESPACE_MW_LABJACK_LJM


#endif /* LabJackLJMQuadratureChannel_hpp */
