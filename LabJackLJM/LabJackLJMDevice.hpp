//
//  LabJackLJMDevice.hpp
//  LabJackLJM
//
//  Created by Christopher Stawarz on 11/20/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#ifndef LabJackLJMDevice_hpp
#define LabJackLJMDevice_hpp

#include "LabJackLJMAnalogChannel.hpp"
#include "LabJackLJMDigitalChannel.hpp"


BEGIN_NAMESPACE_MW_LABJACK_LJM


class Device : public IODevice, boost::noncopyable {
    
public:
    static const std::string DEVICE_TYPE;
    static const std::string CONNECTION_TYPE;
    static const std::string IDENTIFIER;
    static const std::string UPDATE_INTERVAL;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit Device(const ParameterValueMap &parameters);
    ~Device();
    
    void addChild(std::map<std::string, std::string> parameters,
                  ComponentRegistryPtr reg,
                  boost::shared_ptr<Component> child) override;
    
    bool initialize() override;
    bool startDeviceIO() override;
    bool stopDeviceIO() override;
    
private:
    struct IOBuffer {
        void append(const std::string &name, double value = 0.0);
    protected:
        std::vector<std::string> names;
        std::vector<int> addresses;
        std::vector<int> types;
        std::vector<double> values;
    };
    
    struct WriteBuffer : IOBuffer {
        int write(int handle);
    };
    
    struct ReadBuffer : IOBuffer {
        struct ValueReader {
            explicit ValueReader(const std::vector<double> &values) : iter(values.begin()), end(values.end()) { }
            double next();
        private:
            using Iter = std::vector<double>::const_iterator;
            Iter iter;
            const Iter end;
        };
        int read(int handle);
        ValueReader getValues() const { return ValueReader(values); }
    };
    
    static int convertNameToAddress(const std::string &name, int &type);
    static int convertNameToAddress(const std::string &name) {
        int type;
        return convertNameToAddress(name, type);
    }
    
    void reserveLine(const boost::shared_ptr<SingleLineChannel> &channel);
    
    bool haveAnalogInputs() const { return !(analogInputChannels.empty()); }
    void prepareAnalogInputs(WriteBuffer &configBuffer);
    
    bool haveAnalogOutputs() const { return !(analogOutputChannels.empty()); }
    void prepareAnalogOutputs(WriteBuffer &configBuffer);
    void updateAnalogOutputs(WriteBuffer &configBuffer, bool active = false);
    
    bool haveDigitalInputs() const { return !(digitalInputChannels.empty()); }
    void prepareDigitalInputs(WriteBuffer &configBuffer);
    
    bool haveDigitalOutputs() const { return !(digitalOutputChannels.empty()); }
    void prepareDigitalOutputs(WriteBuffer &configBuffer);
    void updateDigitalOutputs(WriteBuffer &configBuffer, bool active = false);
    
    bool haveCounters() const { return !(counterChannels.empty()); }
    void prepareCounters(WriteBuffer &configBuffer);
    bool resetCounters();
    
    bool haveInputs() const { return (haveAnalogInputs() || haveDigitalInputs() || haveCounters()); }
    void startReadInputsTask();
    void stopReadInputsTask();
    void readInputs();
    
    const std::string deviceType;
    const std::string connectionType;
    const std::string identifier;
    const MWTime updateInterval;
    
    const boost::shared_ptr<Clock> clock;
    
    std::vector<boost::shared_ptr<AnalogInputChannel>> analogInputChannels;
    std::vector<boost::shared_ptr<AnalogOutputChannel>> analogOutputChannels;
    
    std::vector<boost::shared_ptr<DigitalInputChannel>> digitalInputChannels;
    std::vector<boost::shared_ptr<DigitalOutputChannel>> digitalOutputChannels;
    
    std::vector<boost::shared_ptr<CounterChannel>> counterChannels;
    
    int handle;
    std::unique_ptr<DeviceInfo> deviceInfo;
    std::set<int> linesInUse;
    
    ReadBuffer inputBuffer;
    boost::shared_ptr<ScheduleTask> readInputsTask;
    
    using lock_guard = std::lock_guard<std::mutex>;
    lock_guard::mutex_type mutex;
    bool running;
    
};


END_NAMESPACE_MW_LABJACK_LJM


#endif /* LabJackLJMDevice_hpp */
