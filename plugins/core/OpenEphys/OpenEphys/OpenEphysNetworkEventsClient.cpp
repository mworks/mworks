//
//  OpenEphysNetworkEventsClient.cpp
//  OpenEphys
//
//  Created by Christopher Stawarz on 5/24/16.
//  Copyright © 2016 The MWorks Project. All rights reserved.
//

#include "OpenEphysNetworkEventsClient.hpp"


BEGIN_NAMESPACE_MW


const std::string OpenEphysNetworkEventsClient::REQUEST("request");
const std::string OpenEphysNetworkEventsClient::RESPONSE("response");


void OpenEphysNetworkEventsClient::describeComponent(ComponentInfo &info) {
    OpenEphysBase::describeComponent(info);
    
    info.setSignature("iodevice/open_ephys_network_events_client");
    
    info.addParameter(REQUEST);
    info.addParameter(RESPONSE);
}


OpenEphysNetworkEventsClient::OpenEphysNetworkEventsClient(const ParameterValueMap &parameters) :
    OpenEphysBase(parameters),
    request(parameters[REQUEST]),
    response(parameters[RESPONSE])
{ }


OpenEphysNetworkEventsClient::~OpenEphysNetworkEventsClient() {
    if (zmqSocket) {
        if (0 != zmq_disconnect(zmqSocket.get(), endpoint.c_str()) &&
            ENOENT != zmq_errno())
        {
            logZMQError("Unable to disconnect from Open Ephys network events module");
        }
    }
}


bool OpenEphysNetworkEventsClient::initialize() {
    zmqSocket.reset(zmq_socket(getZMQContext(), ZMQ_REQ));
    if (!zmqSocket) {
        logZMQError("Unable to create ZeroMQ socket");
        return false;
    }
    
    const int linger = 0;
    const int timeout = 1000;  // ms
    const int immediate = 1;
    if (0 != zmq_setsockopt(zmqSocket.get(), ZMQ_LINGER, &linger, sizeof(linger)) ||
        0 != zmq_setsockopt(zmqSocket.get(), ZMQ_RCVTIMEO, &timeout, sizeof(timeout)) ||
        0 != zmq_setsockopt(zmqSocket.get(), ZMQ_SNDTIMEO, &timeout, sizeof(timeout)) ||
        // Make zmq_send fail if we're not connected (instead of queuing the request for later)
        0 != zmq_setsockopt(zmqSocket.get(), ZMQ_IMMEDIATE, &immediate, sizeof(immediate)))
    {
        logZMQError("Unable to set ZeroMQ socket timeouts");
        return false;
    }
    
    if (0 != zmq_connect(zmqSocket.get(), endpoint.c_str())) {
        logZMQError("Unable to connect to Open Ephys network events module");
        return false;
    }
    
    boost::weak_ptr<OpenEphysNetworkEventsClient> weakThis(component_shared_from_this<OpenEphysNetworkEventsClient>());
    auto notification = [weakThis](const Datum &data, MWTime time) {
        if (auto sharedThis = weakThis.lock()) {
            if (data.isString()) {
                sharedThis->sendRequest(data.getString());
            } else {
                sharedThis->sendRequest(data.toString());
            }
        }
    };
    request->addNotification(boost::make_shared<VariableCallbackNotification>(notification));
    
    return true;
}


void OpenEphysNetworkEventsClient::sendRequest(const std::string &req) {
    if (-1 == zmq_send(zmqSocket.get(), req.data(), req.size(), 0)) {
        logZMQError("Unable to send request to Open Ephys network events module");
        return;
    }
    
    std::vector<char> rep(1024);
    int repSize;
    if (-1 == (repSize = zmq_recv(zmqSocket.get(), rep.data(), rep.size(), 0))) {
        logZMQError("Failed to receive response from Open Ephys network events module");
        return;
    }
    
    response->setValue(Datum(std::string(rep.data(), repSize)));
}


END_NAMESPACE_MW




























