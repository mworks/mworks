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
        (void)incomingSocket.unbind(incomingSocketEndpoint);
        return false;
    }
    
    outgoingConnection.start();
    incomingConnection.start();
    
    return true;
}


bool ZeroMQServer::stop() {
    outgoingConnection.stop();
    bool success = outgoingSocket.unbind(outgoingSocketEndpoint);
    
    incomingConnection.stop();
    success = incomingSocket.unbind(incomingSocketEndpoint) && success;
    
    return success;
}


END_NAMESPACE_MW



























