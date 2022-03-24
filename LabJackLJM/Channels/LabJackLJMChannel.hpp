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
    static const std::string VALUE;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit Channel(const ParameterValueMap &parameters);
    
protected:
    const VariablePtr valueVar;
    
};


class SingleLineChannel : public Channel {
    
public:
    static const std::string LINE;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit SingleLineChannel(const ParameterValueMap &parameters);
    
    virtual void resolveLine(DeviceInfo &deviceInfo);
    
    int getLine() const { return line; }
    const std::string & getCanonicalLineName() const { return canonicalLineName; }
    
protected:
    const std::string lineName;
    
private:
    int line;
    std::string canonicalLineName;
    
};


class MultipleLineChannel : public Channel {
    
public:
    static const std::string FIRST_LINE;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit MultipleLineChannel(const ParameterValueMap &parameters);
    
    virtual void resolveLines(DeviceInfo &deviceInfo);
    
    int getFirstLine() const { return firstLine; }
    
protected:
    const std::string firstLineName;
    
private:
    int firstLine;
    
};


END_NAMESPACE_MW_LABJACK_LJM


#endif /* LabJackLJMChannel_hpp */
