//
//  OpenEphysBase.cpp
//  OpenEphys
//
//  Created by Christopher Stawarz on 5/24/16.
//  Copyright © 2016 The MWorks Project. All rights reserved.
//

#include "OpenEphysBase.hpp"


BEGIN_NAMESPACE_MW


const std::string OpenEphysBase::HOSTNAME("hostname");
const std::string OpenEphysBase::PORT("port");


void OpenEphysBase::describeComponent(ComponentInfo &info) {
    IODevice::describeComponent(info);
    
    info.addParameter(HOSTNAME);
    info.addParameter(PORT);
}


OpenEphysBase::OpenEphysBase(const ParameterValueMap &parameters) :
    IODevice(parameters),
    endpoint("tcp://" + parameters[HOSTNAME].str() + ":" + parameters[PORT].str()),
    zmqSocket(nullptr, zmq_close)
{ }


void * OpenEphysBase::getZMQContext() {
    static std::unique_ptr<void, decltype(&zmq_ctx_term)> zmqContext(zmq_ctx_new(), zmq_ctx_term);
    if (!zmqContext) {
        throw SimpleException(M_IODEVICE_MESSAGE_DOMAIN, "Unable to create ZeroMQ context");
    }
    return zmqContext.get();
}


END_NAMESPACE_MW



























