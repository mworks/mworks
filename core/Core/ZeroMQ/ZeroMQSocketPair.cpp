//
//  ZeroMQSocketPair.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 6/9/21.
//

#include "ZeroMQSocketPair.hpp"

#include "Utilities.h"


BEGIN_NAMESPACE_MW


ZeroMQSocketPair::ZeroMQSocketPair(Type type,
                                   const std::string &incomingSocketEndpoint,
                                   const std::string &outgoingSocketEndpoint) :
    incomingSocket(type == Type::Server ? ZMQ_PULL : ZMQ_SUB),
    outgoingSocket(type == Type::Server ? ZMQ_PUB : ZMQ_PUSH),
    incomingSocketEndpoint(incomingSocketEndpoint),
    outgoingSocketEndpoint(outgoingSocketEndpoint)
{
    switch (type) {
        case Type::Server:
            if (!outgoingSocket.setOption(ZMQ_SNDHWM, 0))  // No limit on number of outstanding outgoing messages
            {
                throw SimpleException(M_NETWORK_MESSAGE_DOMAIN, "Cannot configure ZeroMQ socket");
            }
            break;
            
        case Type::Client:
            if (!incomingSocket.setOption(ZMQ_SUBSCRIBE, "") ||  // Subscribe to all incoming messages
                !incomingSocket.setOption(ZMQ_RCVHWM, 0) ||      // No limit on number of outstanding incoming messages
                !outgoingSocket.setOption(ZMQ_IMMEDIATE, 1))     // Block sends until connection completes
            {
                throw SimpleException(M_NETWORK_MESSAGE_DOMAIN, "Cannot configure ZeroMQ sockets");
            }
            break;
    }
}


bool ZeroMQSocketPair::bind() {
    if (!incomingSocket.bind(incomingSocketEndpoint)) {
        return false;
    }
    
    if (!outgoingSocket.bind(outgoingSocketEndpoint)) {
        // See note on unbinding in ZeroMQSocketPair::unbind
        std::string lastEndpoint;
        if (incomingSocket.getLastEndpoint(lastEndpoint)) {
            (void)incomingSocket.unbind(lastEndpoint);
        }
        return false;
    }
    
    return true;
}


bool ZeroMQSocketPair::unbind() {
    //
    // Unbind each socket from its stored last endpoint, in case the hostname provided to the constructor
    // was a wildcard (see http://api.zeromq.org/master:zmq-tcp for details)
    //
    std::string lastEndpoint;
    auto success = outgoingSocket.getLastEndpoint(lastEndpoint) && outgoingSocket.unbind(lastEndpoint);
    success = incomingSocket.getLastEndpoint(lastEndpoint) && incomingSocket.unbind(lastEndpoint) && success;
    return success;
}


bool ZeroMQSocketPair::connect() {
    if (!outgoingSocket.connect(outgoingSocketEndpoint)) {
        return false;
    }
    
    if (!incomingSocket.connect(incomingSocketEndpoint)) {
        (void)outgoingSocket.disconnect(outgoingSocketEndpoint);
        return false;
    }
    
    return true;
}


bool ZeroMQSocketPair::disconnect() {
    auto success = incomingSocket.disconnect(incomingSocketEndpoint);
    success = outgoingSocket.disconnect(outgoingSocketEndpoint) && success;
    return success;
}


END_NAMESPACE_MW
