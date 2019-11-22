//
//  LabJackLJMChannel.hpp
//  LabJackLJM
//
//  Created by Christopher Stawarz on 11/22/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#ifndef LabJackLJMChannel_hpp
#define LabJackLJMChannel_hpp

#include "LabJackLJMUtilities.hpp"


BEGIN_NAMESPACE_MW_LABJACK_LJM


enum class PhysicalLine {
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
    
    DAC_MIN,
    DAC0 = DAC_MIN,
    DAC1,
    DAC_MAX = DAC1,
    
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

inline constexpr bool isAIN(PhysicalLine line) {
    return (line >= PhysicalLine::AIN_MIN) && (line <= PhysicalLine::AIN_MAX);
}

inline constexpr bool isDAC(PhysicalLine line) {
    return (line >= PhysicalLine::DAC_MIN) && (line <= PhysicalLine::DAC_MAX);
}

inline constexpr bool isFIO(PhysicalLine line) {
    return (line >= PhysicalLine::FIO_MIN) && (line <= PhysicalLine::FIO_MAX);
}

inline constexpr bool isEIO(PhysicalLine line) {
    return (line >= PhysicalLine::EIO_MIN) && (line <= PhysicalLine::EIO_MAX);
}

inline constexpr bool isCIO(PhysicalLine line) {
    return (line >= PhysicalLine::CIO_MIN) && (line <= PhysicalLine::CIO_MAX);
}

inline constexpr bool isMIO(PhysicalLine line) {
    return (line >= PhysicalLine::MIO_MIN) && (line <= PhysicalLine::MIO_MAX);
}

inline constexpr bool isDIO(PhysicalLine line) {
    return (line >= PhysicalLine::DIO_MIN) && (line <= PhysicalLine::DIO_MAX);
}


class Channel : public Component {
    
public:
    explicit Channel(const ParameterValueMap &parameters);
    
protected:
    static PhysicalLine parseLineName(const std::string &name);
    
};


class SingleLineChannel : public Channel {
    
public:
    static const std::string LINE;
    static const std::string VALUE;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit SingleLineChannel(const ParameterValueMap &parameters);
    
    const std::string & getLineName() const { return lineName; }
    PhysicalLine getLine() const { return line; }
    const VariablePtr & getValueVariable() const { return value; }
    
private:
    const std::string lineName;
    const PhysicalLine line;
    const VariablePtr value;
    
};


END_NAMESPACE_MW_LABJACK_LJM


#endif /* LabJackLJMChannel_hpp */
