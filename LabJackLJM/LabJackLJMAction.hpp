//
//  LabJackLJMAction.hpp
//  LabJackLJM
//
//  Created by Christopher Stawarz on 2/16/24.
//  Copyright © 2024 The MWorks Project. All rights reserved.
//

#ifndef LabJackLJMAction_hpp
#define LabJackLJMAction_hpp

#include "LabJackLJMDevice.hpp"


BEGIN_NAMESPACE_MW_LABJACK_LJM


class Action : public mw::Action {
    
public:
    static const std::string DEVICE;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit Action(const ParameterValueMap &parameters);
    
protected:
    boost::shared_ptr<Device> getDevice() const { return weakDevice.lock(); }
    
private:
    boost::weak_ptr<Device> weakDevice;
    
};


class StartAnalogWaveformOutputAction : public Action {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    using Action::Action;
    
    bool execute() override;
    
};


class StopAnalogWaveformOutputAction : public Action {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    using Action::Action;
    
    bool execute() override;
    
};


END_NAMESPACE_MW_LABJACK_LJM


#endif /* LabJackLJMAction_hpp */
