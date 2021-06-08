//
//  ZeroMQClient.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 7/26/16.
//  Copyright © 2016 The MWorks Project. All rights reserved.
//

#include "ZeroMQClient.hpp"


BEGIN_NAMESPACE_MW


ZeroMQClient::ZeroMQClient(const boost::shared_ptr<EventBuffer> &incomingEventBuffer,
                           const boost::shared_ptr<EventBuffer> &outgoingEventBuffer,
                           const std::string &incomingSocketEndpoint,
                           const std::string &outgoingSocketEndpoint) :
    socketPair(ZeroMQSocketPair::Type::Client, incomingSocketEndpoint, outgoingSocketEndpoint),
    incomingConnection(socketPair.getIncomingSocket(), incomingEventBuffer),
    outgoingConnection(socketPair.getOutgoingSocket(), outgoingEventBuffer)
{ }


bool ZeroMQClient::connect() {
    if (!socketPair.connect()) {
        return false;
    }
    incomingConnection.start();
    outgoingConnection.start();
    return true;
}


bool ZeroMQClient::disconnect() {
    incomingConnection.stop();
    outgoingConnection.stop();
    return socketPair.disconnect();
}


END_NAMESPACE_MW
