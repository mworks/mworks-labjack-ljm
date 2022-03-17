//
//  LabJackLJMChannel.hpp
//  LabJackLJM
//
//  Created by Christopher Stawarz on 11/22/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#ifndef LabJackLJMChannel_hpp
#define LabJackLJMChannel_hpp

#include "LabJackLJMDeviceInfo.hpp"


BEGIN_NAMESPACE_MW_LABJACK_LJM


class Channel : public Component {
    
public:
    explicit Channel(const ParameterValueMap &parameters);
    
};


class SingleLineChannel : public Channel {
    
public:
    static const std::string LINE;
    static const std::string VALUE;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit SingleLineChannel(const ParameterValueMap &parameters);
    
    virtual int resolveLine(const DeviceInfo &deviceInfo);
    
    const std::string & getLineName() const { return lineName; }
    const std::string & getCanonicalLineName() const { return canonicalLineName; }
    
protected:
    const std::string lineName;
    const VariablePtr valueVar;
    
    std::string canonicalLineName;
    
};


END_NAMESPACE_MW_LABJACK_LJM


#endif /* LabJackLJMChannel_hpp */
