//
//  LabJackLJMDeviceInfo.hpp
//  LabJackLJM
//
//  Created by Christopher Stawarz on 4/9/20.
//  Copyright © 2020 The MWorks Project. All rights reserved.
//

#ifndef LabJackLJMDeviceInfo_hpp
#define LabJackLJMDeviceInfo_hpp

#include "LabJackLJMUtilities.hpp"


BEGIN_NAMESPACE_MW_LABJACK_LJM


class DeviceInfo {
    
public:
    static std::unique_ptr<DeviceInfo> getDeviceInfo(int deviceType);
    
    virtual ~DeviceInfo() { }
    
    bool reserveLine(int line);
    int reserveLineForName(const std::string &name);
    
    std::string getCanonicalLineName(int line) const;
    
    virtual bool isDAC(int line) const = 0;
    virtual bool isAIN(int line) const = 0;
    virtual bool isDIO(int line) const = 0;
    
    virtual int getDACIndex(int line) const = 0;
    virtual int getAINIndex(int line) const = 0;
    virtual int getDIOIndex(int line) const = 0;
    
    virtual bool isHighSpeedCounter(int line) const = 0;
    virtual bool isInterruptCounter(int line) const = 0;
    virtual bool isQuadraturePhaseA(int line) const = 0;
    
private:
    virtual bool parseLineName(const std::string &name, int &line) const = 0;
    
    std::set<int> linesInUse;
    
};


END_NAMESPACE_MW_LABJACK_LJM


#endif /* LabJackLJMDeviceInfo_hpp */
