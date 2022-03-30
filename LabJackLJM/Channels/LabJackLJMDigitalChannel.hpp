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
    
    void resolveLine(DeviceInfo &deviceInfo) override;
    
    int getDIOIndex() const { return dioIndex; }
    std::uint32_t getDIOBitMask() const { return dioBitMask; }
    
private:
    int dioIndex;
    std::uint32_t dioBitMask;
    
};


class DigitalInputChannel : public DigitalChannel {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    using DigitalChannel::DigitalChannel;
    
    void setValue(bool value, MWTime time) const {
        if (getValueVar()->getValue().getBool() != value) {
            getValueVar()->setValue(Datum(value), time);
        }
    }
    
};


class DigitalOutputChannel : public DigitalChannel {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    using DigitalChannel::DigitalChannel;
    
    void addNewValueNotification(const boost::shared_ptr<VariableNotification> &notification) const {
        getValueVar()->addNotification(notification);
    }
    
    bool getValue() const {
        return getValueVar()->getValue().getBool();
    }
    
};


class WordChannel : public MultipleLineChannel {
    
public:
    static const std::string NUM_LINES;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit WordChannel(const ParameterValueMap &parameters);
    
    void resolveLines(DeviceInfo &deviceInfo) override;
    
    int getFirstLineDIOIndex() const { return firstLineDIOIndex; }
    std::uint32_t getDIOBitMask() const { return dioBitMask; }
    
private:
    const int numLines;
    int firstLineDIOIndex;
    std::uint32_t dioBitMask;
    
};


class WordInputChannel : public WordChannel {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    using WordChannel::WordChannel;
    
    void setValue(long long value, MWTime time) const {
        if (getValueVar()->getValue().getInteger() != value) {
            getValueVar()->setValue(Datum(value), time);
        }
    }
    
};


class WordOutputChannel : public WordChannel {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    using WordChannel::WordChannel;
    
    void addNewValueNotification(const boost::shared_ptr<VariableNotification> &notification) const {
        getValueVar()->addNotification(notification);
    }
    
    long long getValue() const {
        return getValueVar()->getValue().getInteger();
    }
    
};


END_NAMESPACE_MW_LABJACK_LJM


#endif /* LabJackLJMDigitalChannel_hpp */
