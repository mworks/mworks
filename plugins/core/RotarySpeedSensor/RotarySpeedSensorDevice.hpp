//
//  RotarySpeedSensorDevice.hpp
//  RotarySpeedSensor
//
//  Created by Christopher Stawarz on 2/1/24.
//

#ifndef RotarySpeedSensorDevice_hpp
#define RotarySpeedSensorDevice_hpp


BEGIN_NAMESPACE_MW


class RotarySpeedSensorDevice : public IODevice, boost::noncopyable {
    
public:
    static const std::string SERIAL_PORT;
    static const std::string DIRECTION;
    static const std::string RPM;
    static const std::string RECONNECT_INTERVAL;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit RotarySpeedSensorDevice(const ParameterValueMap &parameters);
    ~RotarySpeedSensorDevice();
    
    bool initialize() override;
    bool startDeviceIO() override;
    bool stopDeviceIO() override;
    
private:
    enum {
        START_BYTE_INDEX,
        DIRECTON_BYTE_INDEX,
        RPM_BYTE_INDEX,
        BYTE_INDEX_COUNT
    };
    
    static constexpr speed_t baudRate = B9600;
    
    void receiveData();
    bool reconnect();
    
    const VariablePtr serialPortPath;
    const VariablePtr direction;
    const VariablePtr rpm;
    const MWTime reconnectIntervalUS;
    
    const boost::shared_ptr<Clock> clock;
    
    std::string path;
    SerialPort serialPort;
    
    std::thread receiveDataThread;
    std::atomic_flag continueReceivingData;
    
    std::atomic_bool running;
    static_assert(decltype(running)::is_always_lock_free);
    
};


END_NAMESPACE_MW


#endif /* RotarySpeedSensorDevice_hpp */
