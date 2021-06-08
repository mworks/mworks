//
//  ZeroMQServer.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 8/1/16.
//
//

#include "ZeroMQServer.hpp"

#include "Utilities.h"


BEGIN_NAMESPACE_MW


ZeroMQServer::ZeroMQServer(const boost::shared_ptr<EventBuffer> &incomingEventBuffer,
                           const boost::shared_ptr<EventBuffer> &outgoingEventBuffer,
                           const std::string &incomingSocketEndpoint,
                           const std::string &outgoingSocketEndpoint) :
    incomingSocket(ZMQ_PULL),
    outgoingSocket(ZMQ_PUB),
    incomingConnection(incomingSocket, incomingEventBuffer),
    outgoingConnection(outgoingSocket, outgoingEventBuffer),
    incomingSocketEndpoint(incomingSocketEndpoint),
    outgoingSocketEndpoint(outgoingSocketEndpoint)
{
    if (!outgoingSocket.setOption(ZMQ_SNDHWM, 0))  // No limit on number of outstanding outgoing messages
    {
        throw SimpleException(M_NETWORK_MESSAGE_DOMAIN, "Cannot configure ZeroMQ socket");
    }
}


bool ZeroMQServer::start() {
    if (!incomingSocket.bind(incomingSocketEndpoint)) {
        return false;
    }
    
    if (!outgoingSocket.bind(outgoingSocketEndpoint)) {
        // See note on unbinding in ZeroMQServer::stop
        std::string lastEndpoint;
        if (incomingSocket.getLastEndpoint(lastEndpoint)) {
            (void)incomingSocket.unbind(lastEndpoint);
        }
        return false;
    }
    
    outgoingConnection.start();
    incomingConnection.start();
    
    return true;
}


bool ZeroMQServer::stop() {
    //
    // Unbind each socket from its stored last endpoint, in case the hostname provided to the constructor
    // was a wildcard (see http://api.zeromq.org/4-2:zmq-tcp for details)
    //
    
    outgoingConnection.stop();
    std::string lastEndpoint;
    bool success = outgoingSocket.getLastEndpoint(lastEndpoint) && outgoingSocket.unbind(lastEndpoint);
    
    incomingConnection.stop();
    success = incomingSocket.getLastEndpoint(lastEndpoint) && incomingSocket.unbind(lastEndpoint) && success;
    
    return success;
}


END_NAMESPACE_MW
