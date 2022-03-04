//
//  OpenEphysBase.hpp
//  OpenEphys
//
//  Created by Christopher Stawarz on 5/24/16.
//  Copyright © 2016 The MWorks Project. All rights reserved.
//

#ifndef OpenEphysBase_hpp
#define OpenEphysBase_hpp


BEGIN_NAMESPACE_MW


class OpenEphysBase : public IODevice, boost::noncopyable {
    
public:
    static const std::string HOSTNAME;
    static const std::string PORT;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit OpenEphysBase(const ParameterValueMap &parameters);
    
protected:
    static void * getZMQContext();
    static void logZMQError(const std::string &message) {
        merror(M_IODEVICE_MESSAGE_DOMAIN, "%s: %s", message.c_str(), zmq_strerror(zmq_errno()));
    }
    
    const std::string endpoint;
    std::unique_ptr<void, decltype(&zmq_close)> zmqSocket;
    
};


END_NAMESPACE_MW


#endif /* OpenEphysBase_hpp */



























