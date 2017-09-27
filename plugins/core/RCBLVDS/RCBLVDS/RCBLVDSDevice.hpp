//
//  RCBLVDSDevice.hpp
//  RCBLVDS
//
//  Created by Christopher Stawarz on 9/13/17.
//  Copyright © 2017 The MWorks Project. All rights reserved.
//

#ifndef RCBLVDSDevice_hpp
#define RCBLVDSDevice_hpp

#include "RCBLVDSDigitalInputChannel.hpp"


BEGIN_NAMESPACE_MW


class RCBLVDSDevice : public IODevice, boost::noncopyable {
    
public:
    static const std::string ADDRESS;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit RCBLVDSDevice(const ParameterValueMap &parameters);
    ~RCBLVDSDevice();
    
    void addChild(std::map<std::string, std::string> parameters,
                  ComponentRegistryPtr reg,
                  boost::shared_ptr<Component> child) override;

    bool initialize() override;
    bool startDeviceIO() override;
    bool stopDeviceIO() override;
    
private:
    void updateDigitalInputChannels();
    void setDigitalInputChannels(std::uint16_t state);
    
    bool performRequest(const std::string &path, std::string &responseBody);
    bool performRequest(const std::string &requestBody);
    bool performRequest(const std::string &path, const std::string &requestBody, std::string &responseBody);
    
    const std::string address;
    std::set<std::size_t> activeDigitalInputChannels;
    std::vector<boost::shared_ptr<RCBLVDSDigitalInputChannel>> digitalInputChannels;
    
    NSURLSession *urlSession;
    
    using unique_lock = std::unique_lock<std::mutex>;
    unique_lock::mutex_type mutex;
    bool running;
    
};


END_NAMESPACE_MW


#endif /* RCBLVDSDevice_hpp */





















