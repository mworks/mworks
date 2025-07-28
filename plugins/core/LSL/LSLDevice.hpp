//
//  LSLDevice.hpp
//  LSL
//
//  Created by Christopher Stawarz on 7/29/25.
//

#ifndef LSLDevice_hpp
#define LSLDevice_hpp

#include "lsl_c.h"


BEGIN_NAMESPACE_MW


class LSLDevice : public IODevice, boost::noncopyable {
    
public:
    static const std::string STREAM_INFO;
    static const std::string SAMPLE_DATA;
    static const std::string SAMPLE_TIME;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit LSLDevice(const ParameterValueMap &parameters);
    ~LSLDevice();
    
    bool initialize() override;
    bool startDeviceIO() override;
    bool stopDeviceIO() override;
    
private:
    static constexpr double resolveTimeout = 2.0;
    static constexpr double openTimeout = 1.0;
    static constexpr double pullSampleTimeout = 0.5;
    
    void receiveData();
    
    const VariablePtr streamInfo;
    const VariablePtr sampleData;
    const VariablePtr sampleTime;
    
    const boost::shared_ptr<Clock> clock;
    
    struct InfoDeleter {
        void operator()(lsl_streaminfo info) { lsl_destroy_streaminfo(info); }
    };
    std::unique_ptr<std::remove_pointer_t<lsl_streaminfo>, InfoDeleter> info;
    
    struct InletDeleter {
        void operator()(lsl_inlet inlet) { lsl_destroy_inlet(inlet); }
    };
    std::unique_ptr<std::remove_pointer_t<lsl_inlet>, InletDeleter> inlet;
    
    std::thread receiveDataThread;
    std::atomic_bool running;
    static_assert(decltype(running)::is_always_lock_free);
    
    using lock_guard = std::lock_guard<std::mutex>;
    lock_guard::mutex_type mutex;
    
};


END_NAMESPACE_MW


#endif /* LSLDevice_hpp */
