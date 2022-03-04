//
//  ISCANDevice.hpp
//  ISCAN
//
//  Created by Christopher Stawarz on 5/20/16.
//  Copyright © 2016 The MWorks Project. All rights reserved.
//

#ifndef ISCANDevice_hpp
#define ISCANDevice_hpp


BEGIN_NAMESPACE_MW


class ISCANDevice : public IODevice, boost::noncopyable {
    
public:
    static const std::string SERIAL_PORT;
    static const std::string START_COMMAND;
    static const std::string STOP_COMMAND;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit ISCANDevice(const ParameterValueMap &parameters);
    ~ISCANDevice();
    
    bool initialize() override;
    bool startDeviceIO() override;
    bool stopDeviceIO() override;
    
private:
    static const speed_t baudRate = B115200;
    
    static std::string getOutputParameterName(std::size_t outputNumber);
    
    bool sendCommand(std::uint8_t cmd);
    void receiveData();
    
    const std::string path;
    const int startCommand;
    const int stopCommand;
    SerialPort serialPort;
    
    static const std::size_t maxNumOutputs = 6;
    std::array<VariablePtr, maxNumOutputs> outputs;
    
    using lock_guard = std::lock_guard<std::mutex>;
    lock_guard::mutex_type mutex;
    std::thread receiveDataThread;
    std::atomic_flag continueReceivingData;
    bool running;
    
};


END_NAMESPACE_MW


#endif /* ISCANDevice_hpp */





























