//
//  LabJackLJMDevice.hpp
//  LabJackLJM
//
//  Created by Christopher Stawarz on 11/20/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#ifndef LabJackLJMDevice_hpp
#define LabJackLJMDevice_hpp

#include "LabJackLJMDigitalChannel.hpp"
#include "LabJackLJMDeviceInfo.hpp"


BEGIN_NAMESPACE_MW_LABJACK_LJM


class Device : public IODevice, boost::noncopyable {
    
public:
    static const std::string DEVICE_TYPE;
    static const std::string CONNECTION_TYPE;
    static const std::string IDENTIFIER;
    static const std::string DATA_INTERVAL;
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
    static int convertNameToAddress(const std::string &name, int &type);
    static int convertNameToAddress(const std::string &name) {
        int type;
        return convertNameToAddress(name, type);
    }
    
    int reserveLine(const std::string &lineName);
    void validateDigitalChannel(const boost::shared_ptr<DigitalChannel> &channel);
    
    bool haveDigitalInputs() const { return !(digitalInputChannels.empty()); }
    void prepareDigitalInputs();
    
    bool haveDigitalOutputs() const { return !(digitalOutputChannels.empty()); }
    void prepareDigitalOutputs();
    void updateDigitalOutputs(bool active = false);
    
    bool haveInputs() const { return (haveDigitalInputs()); }
    void readInputs();
    
    struct WriteBuffer {
        explicit WriteBuffer(int &handle) : handle(handle) { }
        void append(const std::string &name, double value);
        int write();
    private:
        int &handle;
        std::vector<std::string> names;
        std::vector<int> addresses;
        std::vector<int> types;
        std::vector<double> values;
    };
    
    struct Stream {
        struct DataReader {
            explicit DataReader(const std::vector<double> &data) : iter(data.begin()), end(data.end()) { }
            template<typename T> T get();
            explicit operator bool() const noexcept { return (iter != end); }
        private:
            double next() { return *(iter++); }
            using Iter = std::vector<double>::const_iterator;
            Iter iter;
            const Iter end;
        };
        explicit Stream(Device &device) : device(device) { }
        void add(const std::string &name);
        bool start();
        bool read();
        bool stop();
        DataReader getData() const { return DataReader(data); }
    private:
        static void callback(void *_device);
        Device &device;
        std::vector<int> addresses;
        int scansPerRead = 0;
        std::vector<double> data;
    };
    
    const std::string deviceType;
    const std::string connectionType;
    const std::string identifier;
    const MWTime dataInterval;
    const MWTime updateInterval;
    
    const boost::shared_ptr<Clock> clock;
    
    std::vector<boost::shared_ptr<DigitalInputChannel>> digitalInputChannels;
    std::vector<boost::shared_ptr<DigitalOutputChannel>> digitalOutputChannels;
    
    int handle;
    std::unique_ptr<DeviceInfo> deviceInfo;
    std::set<int> linesInUse;
    WriteBuffer writeBuffer;
    Stream stream;
    
    using lock_guard = std::lock_guard<std::mutex>;
    lock_guard::mutex_type mutex;
    bool running;
    
};


END_NAMESPACE_MW_LABJACK_LJM


#endif /* LabJackLJMDevice_hpp */
