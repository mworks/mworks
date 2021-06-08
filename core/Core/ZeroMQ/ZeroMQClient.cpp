//
//  ZeroMQClient.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 7/26/16.
//  Copyright © 2016 The MWorks Project. All rights reserved.
//

#include "ZeroMQClient.hpp"

#include "Utilities.h"


BEGIN_NAMESPACE_MW


ZeroMQClient::ZeroMQClient(const boost::shared_ptr<EventBuffer> &incomingEventBuffer,
                           const boost::shared_ptr<EventBuffer> &outgoingEventBuffer,
                           const std::string &incomingSocketEndpoint,
                           const std::string &outgoingSocketEndpoint) :
    incomingSocket(ZMQ_SUB),
    outgoingSocket(ZMQ_PUSH),
    incomingConnection(incomingSocket, incomingEventBuffer),
    outgoingConnection(outgoingSocket, outgoingEventBuffer),
    incomingSocketEndpoint(incomingSocketEndpoint),
    outgoingSocketEndpoint(outgoingSocketEndpoint)
{
    if (!incomingSocket.setOption(ZMQ_SUBSCRIBE, "") ||  // Subscribe to all incoming messages
        !incomingSocket.setOption(ZMQ_RCVHWM, 0) ||      // No limit on number of outstanding incoming messages
        !outgoingSocket.setOption(ZMQ_IMMEDIATE, 1))     // Block sends until connection completes
    {
        throw SimpleException(M_NETWORK_MESSAGE_DOMAIN, "Cannot configure ZeroMQ sockets");
    }
}


bool ZeroMQClient::connect() {
    if (!outgoingSocket.connect(outgoingSocketEndpoint)) {
        return false;
    }
    
    if (!incomingSocket.connect(incomingSocketEndpoint)) {
        (void)outgoingSocket.disconnect(outgoingSocketEndpoint);
        return false;
    }
    
    incomingConnection.start();
    outgoingConnection.start();
    
    return true;
}


bool ZeroMQClient::disconnect() {
    incomingConnection.stop();
    bool success = incomingSocket.disconnect(incomingSocketEndpoint);
    
    outgoingConnection.stop();
    success = outgoingSocket.disconnect(outgoingSocketEndpoint) && success;
    
    return success;
}


END_NAMESPACE_MW
