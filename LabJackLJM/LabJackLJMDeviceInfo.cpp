//
//  LabJackLJMDeviceInfo.cpp
//  LabJackLJM
//
//  Created by Christopher Stawarz on 4/9/20.
//  Copyright © 2020 The MWorks Project. All rights reserved.
//

#include "LabJackLJMDeviceInfo.hpp"


BEGIN_NAMESPACE_MW_LABJACK_LJM


class T7DeviceInfo : public DeviceInfo {
    
    enum class PhysicalLine {
        DAC_MIN,
        DAC0 = DAC_MIN,
        DAC1,
        DAC_MAX = DAC1,
        
        AIN_MIN,
        AIN0 = AIN_MIN,
        AIN1,
        AIN2,
        AIN3,
        AIN4,
        AIN5,
        AIN6,
        AIN7,
        AIN8,
        AIN9,
        AIN10,
        AIN11,
        AIN12,
        AIN13,
        AIN_MAX = AIN13,
        
        FIO_MIN,
        FIO0 = FIO_MIN,
        FIO1,
        FIO2,
        FIO3,
        FIO4,
        FIO5,
        FIO6,
        FIO7,
        FIO_MAX = FIO7,
        
        EIO_MIN,
        EIO0 = EIO_MIN,
        EIO1,
        EIO2,
        EIO3,
        EIO4,
        EIO5,
        EIO6,
        EIO7,
        EIO_MAX = EIO7,
        
        CIO_MIN,
        CIO0 = CIO_MIN,
        CIO1,
        CIO2,
        CIO3,
        CIO_MAX = CIO3,
        
        MIO_MIN,
        MIO0 = MIO_MIN,
        MIO1,
        MIO2,
        MIO_MAX = MIO2,
        
        DIO_MIN = FIO_MIN,
        DIO0 = DIO_MIN,
        DIO1,
        DIO2,
        DIO3,
        DIO4,
        DIO5,
        DIO6,
        DIO7,
        DIO8,
        DIO9,
        DIO10,
        DIO11,
        DIO12,
        DIO13,
        DIO14,
        DIO15,
        DIO16,
        DIO17,
        DIO18,
        DIO19,
        DIO20,
        DIO21,
        DIO22,
        DIO_MAX = DIO22
    };
    
    static_assert(PhysicalLine::DIO_MAX == PhysicalLine::MIO_MAX);  // Sanity check
    
    template<PhysicalLine rangeMin, PhysicalLine rangeMax>
    static bool isInRange(int line) {
        return (line >= static_cast<int>(rangeMin)) && (line <= static_cast<int>(rangeMax));
    }
    
    bool isDAC(int line) const override {
        return isInRange<PhysicalLine::DAC_MIN, PhysicalLine::DAC_MAX>(line);
    }
    
    bool isAIN(int line) const override {
        return isInRange<PhysicalLine::AIN_MIN, PhysicalLine::AIN_MAX>(line);
    }
    
    bool isDIO(int line) const override {
        return isInRange<PhysicalLine::DIO_MIN, PhysicalLine::DIO_MAX>(line);
    }
    
    int getDACIndex(int line) const override {
        return (line - static_cast<int>(PhysicalLine::DAC0));
    }
    
    int getAINIndex(int line) const override {
        return (line - static_cast<int>(PhysicalLine::AIN0));
    }
    
    int getDIOIndex(int line) const override {
        return (line - static_cast<int>(PhysicalLine::DIO0));
    }
    
    bool isHighSpeedCounter(int line) const override {
        return isInRange<PhysicalLine::CIO0, PhysicalLine::CIO3>(line);
    }
    
    bool isQuadraturePhaseA(int line) const override {
        return (line == static_cast<int>(PhysicalLine::DIO0) ||
                line == static_cast<int>(PhysicalLine::DIO2) ||
                line == static_cast<int>(PhysicalLine::DIO6));
    }
    
    bool parseLineName(const std::string &name, int &line) const override;
    
};


class T4DeviceInfo : public DeviceInfo {
    
    enum class PhysicalLine {
        DAC_MIN,
        DAC0 = DAC_MIN,
        DAC1,
        DAC_MAX = DAC1,
        
        AIN_MIN,
        AIN0 = AIN_MIN,
        AIN1,
        AIN2,
        AIN3,
        AIN_MAX = AIN3,
        
        FIO_MIN,
        FIO4 = FIO_MIN,
        FIO5,
        FIO6,
        FIO7,
        FIO_MAX = FIO7,
        
        EIO_MIN,
        EIO0 = EIO_MIN,
        EIO1,
        EIO2,
        EIO3,
        EIO4,
        EIO5,
        EIO6,
        EIO7,
        EIO_MAX = EIO7,
        
        CIO_MIN,
        CIO0 = CIO_MIN,
        CIO1,
        CIO2,
        CIO3,
        CIO_MAX = CIO3,
        
        DIO_MIN = FIO_MIN,
        DIO4 = DIO_MIN,
        DIO5,
        DIO6,
        DIO7,
        DIO8,
        DIO9,
        DIO10,
        DIO11,
        DIO12,
        DIO13,
        DIO14,
        DIO15,
        DIO16,
        DIO17,
        DIO18,
        DIO19,
        DIO_MAX = DIO19
    };
    
    static_assert(PhysicalLine::DIO_MAX == PhysicalLine::CIO_MAX);  // Sanity check
    
    template<PhysicalLine rangeMin, PhysicalLine rangeMax>
    static bool isInRange(int line) {
        return (line >= static_cast<int>(rangeMin)) && (line <= static_cast<int>(rangeMax));
    }
    
    bool isDAC(int line) const override {
        return isInRange<PhysicalLine::DAC_MIN, PhysicalLine::DAC_MAX>(line);
    }
    
    bool isAIN(int line) const override {
        return isInRange<PhysicalLine::AIN_MIN, PhysicalLine::AIN_MAX>(line);
    }
    
    bool isDIO(int line) const override {
        return isInRange<PhysicalLine::DIO_MIN, PhysicalLine::DIO_MAX>(line);
    }
    
    int getDACIndex(int line) const override {
        return (line - static_cast<int>(PhysicalLine::DAC0));
    }
    
    int getAINIndex(int line) const override {
        return (line - static_cast<int>(PhysicalLine::AIN0));
    }
    
    int getDIOIndex(int line) const override {
        return (line - static_cast<int>(PhysicalLine::DIO4) + 4);  // No DIO0-3 on T4
    }
    
    bool isHighSpeedCounter(int line) const override {
        return isInRange<PhysicalLine::CIO0, PhysicalLine::CIO3>(line);
    }
    
    bool isQuadraturePhaseA(int line) const override {
        return (line == static_cast<int>(PhysicalLine::DIO4) ||
                line == static_cast<int>(PhysicalLine::DIO6) ||
                line == static_cast<int>(PhysicalLine::DIO8));
    }
    
    bool parseLineName(const std::string &name, int &line) const override;
    
};


std::unique_ptr<DeviceInfo> DeviceInfo::getDeviceInfo(int deviceType) {
    switch (deviceType) {
        case LJM_dtT7:
            return std::make_unique<T7DeviceInfo>();
        case LJM_dtT4:
            return std::make_unique<T4DeviceInfo>();
        default:
            return nullptr;
    }
}


bool DeviceInfo::reserveLine(int line) {
    return linesInUse.insert(line).second;
}


int DeviceInfo::reserveLineForName(const std::string &name) {
    int line;
    if (!parseLineName(boost::algorithm::to_upper_copy(name), line)) {
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN,
                              boost::format("Invalid line name for current LabJack LJM device: \"%s\"") % name);
    }
    if (!reserveLine(line)) {
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN, boost::format("Line %s is already in use") % name);
    }
    return line;
}


std::string DeviceInfo::getCanonicalLineName(int line) const {
    if (isDAC(line)) {
        return "DAC" + std::to_string(getDACIndex(line));
    }
    if (isAIN(line)) {
        return "AIN" + std::to_string(getAINIndex(line));
    }
    if (isDIO(line)) {
        return "DIO" + std::to_string(getDIOIndex(line));
    }
    return "";
}


bool T7DeviceInfo::parseLineName(const std::string &name, int &line) const {
    static const std::map<std::string, PhysicalLine> validNames {
        { "DAC0", PhysicalLine::DAC0 },
        { "DAC1", PhysicalLine::DAC1 },
        { "AIN0", PhysicalLine::AIN0 },
        { "AIN1", PhysicalLine::AIN1 },
        { "AIN2", PhysicalLine::AIN2 },
        { "AIN3", PhysicalLine::AIN3 },
        { "AIN4", PhysicalLine::AIN4 },
        { "AIN5", PhysicalLine::AIN5 },
        { "AIN6", PhysicalLine::AIN6 },
        { "AIN7", PhysicalLine::AIN7 },
        { "AIN8", PhysicalLine::AIN8 },
        { "AIN9", PhysicalLine::AIN9 },
        { "AIN10", PhysicalLine::AIN10 },
        { "AIN11", PhysicalLine::AIN11 },
        { "AIN12", PhysicalLine::AIN12 },
        { "AIN13", PhysicalLine::AIN13 },
        { "FIO0", PhysicalLine::FIO0 },
        { "FIO1", PhysicalLine::FIO1 },
        { "FIO2", PhysicalLine::FIO2 },
        { "FIO3", PhysicalLine::FIO3 },
        { "FIO4", PhysicalLine::FIO4 },
        { "FIO5", PhysicalLine::FIO5 },
        { "FIO6", PhysicalLine::FIO6 },
        { "FIO7", PhysicalLine::FIO7 },
        { "EIO0", PhysicalLine::EIO0 },
        { "EIO1", PhysicalLine::EIO1 },
        { "EIO2", PhysicalLine::EIO2 },
        { "EIO3", PhysicalLine::EIO3 },
        { "EIO4", PhysicalLine::EIO4 },
        { "EIO5", PhysicalLine::EIO5 },
        { "EIO6", PhysicalLine::EIO6 },
        { "EIO7", PhysicalLine::EIO7 },
        { "CIO0", PhysicalLine::CIO0 },
        { "CIO1", PhysicalLine::CIO1 },
        { "CIO2", PhysicalLine::CIO2 },
        { "CIO3", PhysicalLine::CIO3 },
        { "MIO0", PhysicalLine::MIO0 },
        { "MIO1", PhysicalLine::MIO1 },
        { "MIO2", PhysicalLine::MIO2 },
        { "DIO0", PhysicalLine::DIO0 },
        { "DIO1", PhysicalLine::DIO1 },
        { "DIO2", PhysicalLine::DIO2 },
        { "DIO3", PhysicalLine::DIO3 },
        { "DIO4", PhysicalLine::DIO4 },
        { "DIO5", PhysicalLine::DIO5 },
        { "DIO6", PhysicalLine::DIO6 },
        { "DIO7", PhysicalLine::DIO7 },
        { "DIO8", PhysicalLine::DIO8 },
        { "DIO9", PhysicalLine::DIO9 },
        { "DIO10", PhysicalLine::DIO10 },
        { "DIO11", PhysicalLine::DIO11 },
        { "DIO12", PhysicalLine::DIO12 },
        { "DIO13", PhysicalLine::DIO13 },
        { "DIO14", PhysicalLine::DIO14 },
        { "DIO15", PhysicalLine::DIO15 },
        { "DIO16", PhysicalLine::DIO16 },
        { "DIO17", PhysicalLine::DIO17 },
        { "DIO18", PhysicalLine::DIO18 },
        { "DIO19", PhysicalLine::DIO19 },
        { "DIO20", PhysicalLine::DIO20 },
        { "DIO21", PhysicalLine::DIO21 },
        { "DIO22", PhysicalLine::DIO22 },
    };
    
    auto iter = validNames.find(name);
    if (iter != validNames.end()) {
        line = static_cast<int>(iter->second);
        return true;
    }
    return false;
}


bool T4DeviceInfo::parseLineName(const std::string &name, int &line) const {
    static const std::map<std::string, PhysicalLine> validNames {
        { "DAC0", PhysicalLine::DAC0 },
        { "DAC1", PhysicalLine::DAC1 },
        { "AIN0", PhysicalLine::AIN0 },
        { "AIN1", PhysicalLine::AIN1 },
        { "AIN2", PhysicalLine::AIN2 },
        { "AIN3", PhysicalLine::AIN3 },
        { "FIO4", PhysicalLine::FIO4 },
        { "FIO5", PhysicalLine::FIO5 },
        { "FIO6", PhysicalLine::FIO6 },
        { "FIO7", PhysicalLine::FIO7 },
        { "EIO0", PhysicalLine::EIO0 },
        { "EIO1", PhysicalLine::EIO1 },
        { "EIO2", PhysicalLine::EIO2 },
        { "EIO3", PhysicalLine::EIO3 },
        { "EIO4", PhysicalLine::EIO4 },
        { "EIO5", PhysicalLine::EIO5 },
        { "EIO6", PhysicalLine::EIO6 },
        { "EIO7", PhysicalLine::EIO7 },
        { "CIO0", PhysicalLine::CIO0 },
        { "CIO1", PhysicalLine::CIO1 },
        { "CIO2", PhysicalLine::CIO2 },
        { "CIO3", PhysicalLine::CIO3 },
        { "DIO4", PhysicalLine::DIO4 },
        { "DIO5", PhysicalLine::DIO5 },
        { "DIO6", PhysicalLine::DIO6 },
        { "DIO7", PhysicalLine::DIO7 },
        { "DIO8", PhysicalLine::DIO8 },
        { "DIO9", PhysicalLine::DIO9 },
        { "DIO10", PhysicalLine::DIO10 },
        { "DIO11", PhysicalLine::DIO11 },
        { "DIO12", PhysicalLine::DIO12 },
        { "DIO13", PhysicalLine::DIO13 },
        { "DIO14", PhysicalLine::DIO14 },
        { "DIO15", PhysicalLine::DIO15 },
        { "DIO16", PhysicalLine::DIO16 },
        { "DIO17", PhysicalLine::DIO17 },
        { "DIO18", PhysicalLine::DIO18 },
        { "DIO19", PhysicalLine::DIO19 },
    };
    
    auto iter = validNames.find(name);
    if (iter != validNames.end()) {
        line = static_cast<int>(iter->second);
        return true;
    }
    return false;
}


END_NAMESPACE_MW_LABJACK_LJM
