//
//  EasyEyeDevice.hpp
//  EasyEye
//
//  Created by Christopher Stawarz on 7/25/17.
//  Copyright © 2017 The MWorks Project. All rights reserved.
//

#ifndef EasyEyeDevice_hpp
#define EasyEyeDevice_hpp


BEGIN_NAMESPACE_MW


class EasyEyeDevice : public IODevice {
    
public:
    static const std::string ADDRESS;
    static const std::string PORT;
    static const std::string EYE_X;
    static const std::string EYE_Y;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit EasyEyeDevice(const ParameterValueMap &parameters);
    ~EasyEyeDevice();
    
    bool initialize() override;
    bool startDeviceIO() override;
    bool stopDeviceIO() override;
    
private:
    void receiveData();
    
    const std::string socketEndpoint;
    const VariablePtr eyeX;
    const VariablePtr eyeY;
    
    ZeroMQSocket socket;
    std::thread receiveDataThread;
    std::atomic_flag continueReceivingData;
    
    static_assert(ATOMIC_BOOL_LOCK_FREE == 2, "std::atomic_bool is not always lock-free");
    std::atomic_bool running;
    
};


END_NAMESPACE_MW


#endif /* EasyEyeDevice_hpp */



























