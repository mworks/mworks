//
//  ZeroMQIPCEventTransport.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 6/8/21.
//

#include "ZeroMQIPCEventTransport.hpp"

#include "Utilities.h"


BEGIN_NAMESPACE_MW


ZeroMQIPCEventTransport::ZeroMQIPCEventTransport(event_transport_type type, const std::string &resourceName) :
    EventTransport(type, bidirectional_event_transport)
{
    // Create socket endpoints
    const auto socketEndpointBase = "ipc:///tmp/mworks_zmq_ipc_" + resourceName;
    const auto incomingSocketEndpoint = socketEndpointBase + "_in";
    const auto outgoingSocketEndpoint = socketEndpointBase + "_out";
    
    // Create socket pair
    switch (type) {
        case server_event_transport:
            socketPair = std::make_unique<ZeroMQSocketPair>(ZeroMQSocketPair::Type::Server,
                                                            incomingSocketEndpoint,
                                                            outgoingSocketEndpoint);
            break;
            
        case client_event_transport:
            socketPair = std::make_unique<ZeroMQSocketPair>(ZeroMQSocketPair::Type::Client,
                                                            outgoingSocketEndpoint,
                                                            incomingSocketEndpoint);
            break;
            
        default:
            throw SimpleException(M_NETWORK_MESSAGE_DOMAIN,
                                  "Internal error: invalid event_transport_type in ZeroMQIPCEventTransport constructor");
    }
    
    // Set receive and send timeouts
    if (!socketPair->getIncomingSocket().setOption(ZMQ_RCVTIMEO, 0) ||   // Receives time out immediately (no blocking)
        !socketPair->getOutgoingSocket().setOption(ZMQ_SNDTIMEO, 2000))  // Sends time out after 2 seconds
    {
        throw SimpleException(M_NETWORK_MESSAGE_DOMAIN, "Cannot configure ZeroMQ sockets");
    }
    
    // Bind/connect sockets
    switch (type) {
        case server_event_transport:
            if (!socketPair->bind()) {
                throw SimpleException(M_NETWORK_MESSAGE_DOMAIN,
                                      boost::format("Cannot bind server event transport with resource name \"%1%\"")
                                      % resourceName);
            }
            break;
            
        case client_event_transport:
            if (!socketPair->connect()) {
                throw SimpleException(M_NETWORK_MESSAGE_DOMAIN,
                                      boost::format("Cannot connect client event transport with resource name \"%1%\"")
                                      % resourceName);
            }
            break;
            
        default:
            break;
    }
}


ZeroMQIPCEventTransport::~ZeroMQIPCEventTransport() {
    switch (type) {
        case server_event_transport:
            (void)socketPair->unbind();
            break;
            
        case client_event_transport:
            (void)socketPair->disconnect();
            break;
            
        default:
            break;
    }
}


void ZeroMQIPCEventTransport::sendEvent(const boost::shared_ptr<Event> &event) {
    switch (socketPair->send(event)) {
        case ZeroMQSocket::Result::timeout:
            merror(M_NETWORK_MESSAGE_DOMAIN, "Send timed out on IPC socket");
            break;
            
        case ZeroMQSocket::Result::error:
            merror(M_NETWORK_MESSAGE_DOMAIN, "Cannot send event on IPC socket");
            break;
            
        default:
            break;
    }
}


boost::shared_ptr<Event> ZeroMQIPCEventTransport::receiveEventAsynchronous() {
    boost::shared_ptr<Event> event;
    if (ZeroMQSocket::Result::error == socketPair->recv(event)) {
        merror(M_NETWORK_MESSAGE_DOMAIN, "Cannot receive event on IPC socket");
    }
    return event;
}


END_NAMESPACE_MW
