//
//  FirmataDevice.hpp
//  Firmata
//
//  Created by Christopher Stawarz on 6/10/16.
//  Copyright © 2016 The MWorks Project. All rights reserved.
//

#ifndef FirmataDevice_hpp
#define FirmataDevice_hpp

#include "FirmataDigitalChannel.hpp"


BEGIN_NAMESPACE_MW


class FirmataDevice : public IODevice, boost::noncopyable {
    
public:
    static const std::string SERIAL_PORT;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit FirmataDevice(const ParameterValueMap &parameters);
    ~FirmataDevice();
    
    void addChild(std::map<std::string, std::string> parameters,
                  ComponentRegistryPtr reg,
                  boost::shared_ptr<Component> child) override;
    
    bool initialize() override;
    bool startDeviceIO() override;
    bool stopDeviceIO() override;
    
private:
    using unique_lock = std::unique_lock<std::mutex>;
    
    static constexpr std::uint8_t protocolVersionMajor = 2;
    static constexpr std::uint8_t protocolVersionMinor = 5;
    
    static constexpr std::size_t numPorts = 16;
    static constexpr std::size_t numPinsPerPort = 8;
    
    boost::shared_ptr<FirmataDigitalChannel>& getChannelForPin(int pinNumber) {
        return ports.at(pinNumber / numPinsPerPort).at(pinNumber % numPinsPerPort);
    }
    
    bool checkProtocolVersion(unique_lock &lock);
    bool configureDigitalPorts();
    bool startDigitalIO();
    bool stopDigitalIO();
    void setDigitalOutput(int pinNumber, bool value);
    bool sendData(const std::vector<std::uint8_t> &data);
    void receiveData();
    
    const std::string path;
    std::array<std::array<boost::shared_ptr<FirmataDigitalChannel>, numPinsPerPort>, numPorts> ports;
    SerialPort serialPort;
    
    unique_lock::mutex_type mutex;
    std::condition_variable condition;
    std::thread receiveDataThread;
    std::atomic_flag continueReceivingData;
    
    bool deviceProtocolVersionReceived;
    std::uint8_t deviceProtocolVersionMajor;
    std::uint8_t deviceProtocolVersionMinor;
    
    bool running;
    
};


END_NAMESPACE_MW


#endif /* FirmataDevice_hpp */




























