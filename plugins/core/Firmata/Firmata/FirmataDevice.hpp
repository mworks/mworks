//
//  FirmataDevice.hpp
//  Firmata
//
//  Created by Christopher Stawarz on 6/10/16.
//  Copyright © 2016 The MWorks Project. All rights reserved.
//

#ifndef FirmataDevice_hpp
#define FirmataDevice_hpp

#include "FirmataChannel.hpp"
#include "FirmataConnection.hpp"


BEGIN_NAMESPACE_MW


class FirmataDevice : public IODevice, boost::noncopyable {
    
public:
    static const std::string SERIAL_PORT;
    static const std::string BLUETOOTH_LOCAL_NAME;
    static const std::string DATA_INTERVAL;
    
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
    
    static int getPinNumber(std::size_t portNum, std::size_t bitNum) {
        return portNum * numPinsPerPort + bitNum;
    }
    
    boost::shared_ptr<FirmataChannel>& getChannelForPin(int pinNumber) {
        return ports.at(pinNumber / numPinsPerPort).at(pinNumber % numPinsPerPort);
    }
    
    int getAnalogChannelNumber(int pinNumber) const {
        for (auto &item : pinForAnalogChannel) {
            if (item.second == pinNumber) {
                return item.first;
            }
        }
        return -1;
    }
    
    bool checkProtocolVersion(unique_lock &lock);
    bool getDeviceInfo(unique_lock &lock);
    bool processChannelRequests();
    bool configurePins();
    bool startIO();
    bool stopIO();
    int getResolutionForPinMode(int pinNumber, int pinMode) const;
    double getMaximumValueForPinMode(int pinNumber, int pinMode) const;
    bool setAnalogOutput(int pinNumber, double value);
    bool setDigitalOutput(int pinNumber, bool value);
    bool setServo(int pinNumber, double value);
    bool sendExtendedAnalogMessage(int pinNumber, int pinMode, int value);
    bool sendData(const std::vector<std::uint8_t> &data);
    void receiveData();
    
    const std::unique_ptr<FirmataConnection> connection;
    MWTime samplingIntervalUS;
    std::vector<boost::shared_ptr<FirmataChannel>> requestedChannels;
    std::array<std::array<boost::shared_ptr<FirmataChannel>, numPinsPerPort>, numPorts> ports;
    
    unique_lock::mutex_type mutex;
    std::condition_variable condition;
    std::thread receiveDataThread;
    std::atomic_flag continueReceivingData;
    
    bool deviceProtocolVersionReceived;
    std::uint8_t deviceProtocolVersionMajor;
    std::uint8_t deviceProtocolVersionMinor;
    
    bool capabilityInfoReceived;
    std::map<std::uint8_t, std::map<std::uint8_t, std::uint8_t>> modesForPin;
    
    bool analogMappingInfoReceived;
    std::map<std::uint8_t, std::uint8_t> pinForAnalogChannel;
    
    static_assert(ATOMIC_BOOL_LOCK_FREE == 2, "std::atomic_bool is not always lock-free");
    std::atomic_bool running;
    
};


END_NAMESPACE_MW


#endif /* FirmataDevice_hpp */




























