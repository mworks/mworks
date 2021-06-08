//
//  ZeroMQServer.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 8/1/16.
//
//

#include "ZeroMQServer.hpp"


BEGIN_NAMESPACE_MW


ZeroMQServer::ZeroMQServer(const boost::shared_ptr<EventBuffer> &incomingEventBuffer,
                           const boost::shared_ptr<EventBuffer> &outgoingEventBuffer,
                           const std::string &incomingSocketEndpoint,
                           const std::string &outgoingSocketEndpoint) :
    socketPair(ZeroMQSocketPair::Type::Server, incomingSocketEndpoint, outgoingSocketEndpoint),
    incomingConnection(socketPair.getIncomingSocket(), incomingEventBuffer),
    outgoingConnection(socketPair.getOutgoingSocket(), outgoingEventBuffer)
{ }


bool ZeroMQServer::start() {
    if (!socketPair.bind()) {
        return false;
    }
    outgoingConnection.start();
    incomingConnection.start();
    return true;
}


bool ZeroMQServer::stop() {
    outgoingConnection.stop();
    incomingConnection.stop();
    return socketPair.unbind();
}


END_NAMESPACE_MW
