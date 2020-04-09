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
    
    void reserveLine(const std::string &lineName, PhysicalLine line);
    
    void prepareDigitalInput();
    
    void prepareDigitalOutput();
    void updateDigitalOutput(bool active = false);
    
    struct WriteBuffer {
        void append(const std::string &name, double value);
        int write(int handle);
    private:
        std::vector<int> addresses;
        std::vector<int> types;
        std::vector<double> values;
    };
    
    struct Stream {
        void add(const std::string &name);
    private:
        std::vector<int> addresses;
    };
    
    const std::string deviceType;
    const std::string connectionType;
    const std::string identifier;
    const MWTime dataInterval;
    const MWTime updateInterval;
    
    std::set<PhysicalLine> linesInUse;
    std::vector<boost::shared_ptr<DigitalInputChannel>> digitalInputChannels;
    std::vector<boost::shared_ptr<DigitalOutputChannel>> digitalOutputChannels;
    
    int handle;
    std::unique_ptr<DeviceInfo> deviceInfo;
    WriteBuffer writeBuffer;
    Stream stream;
    
    using lock_guard = std::lock_guard<std::mutex>;
    lock_guard::mutex_type mutex;
    bool running;
    
};


END_NAMESPACE_MW_LABJACK_LJM


#endif /* LabJackLJMDevice_hpp */
