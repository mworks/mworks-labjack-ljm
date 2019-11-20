//
//  LabJackLJMDevice.hpp
//  LabJackLJM
//
//  Created by Christopher Stawarz on 11/20/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#ifndef LabJackLJMDevice_hpp
#define LabJackLJMDevice_hpp


BEGIN_NAMESPACE_MW


class LabJackLJMDevice : public IODevice, boost::noncopyable {
    
public:
    static const std::string DEVICE_TYPE;
    static const std::string CONNECTION_TYPE;
    static const std::string IDENTIFIER;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit LabJackLJMDevice(const ParameterValueMap &parameters);
    ~LabJackLJMDevice();
    
    bool initialize() override;
    
private:
    const std::string deviceType;
    const std::string connectionType;
    const std::string identifier;
    
    int handle;
    
};


END_NAMESPACE_MW


#endif /* LabJackLJMDevice_hpp */
