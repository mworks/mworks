//
//  ZeroMQConnection.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 7/29/16.
//
//

#include "ZeroMQConnection.hpp"

#include <boost/format.hpp>

#include "Clock.h"


BEGIN_NAMESPACE_MW


ZeroMQIncomingConnectionBase::ZeroMQIncomingConnectionBase(ZeroMQSocket &socket,
                                                           const boost::shared_ptr<EventBuffer> &eventBuffer) :
    ZeroMQConnectionBase(socket, eventBuffer)
{
    if (!socket.setOption(ZMQ_RCVTIMEO, receiveTimeoutMS)) {
        throw SimpleException(M_NETWORK_MESSAGE_DOMAIN, "Cannot configure ZeroMQ socket");
    }
}


std::thread ZeroMQIncomingConnectionBase::startEventHandlerThread() {
    return std::thread([this]() { handleEvents(); });
}


void ZeroMQIncomingConnectionBase::handleEvents() {
    while (running) {
        boost::shared_ptr<Event> event;
        switch (socket.recv(event)) {
            case ZeroMQSocket::Result::ok:
                eventBuffer->putEvent(event);
                break;
                
            case ZeroMQSocket::Result::error:
                merror(M_NETWORK_MESSAGE_DOMAIN, "Terminating incoming connection");
                running = false;
                return;
                
            default:
                break;
        }
    }
}


ZeroMQOutgoingConnectionBase::ZeroMQOutgoingConnectionBase(ZeroMQSocket &socket,
                                                           const boost::shared_ptr<EventBuffer> &eventBuffer) :
    ZeroMQConnectionBase(socket, eventBuffer)
{
    if (!socket.setOption(ZMQ_SNDTIMEO, sendTimeoutMS)) {
        throw SimpleException(M_NETWORK_MESSAGE_DOMAIN, "Cannot configure ZeroMQ socket");
    }
}


std::thread ZeroMQOutgoingConnectionBase::startEventHandlerThread() {
    // Create the buffer reader here, so that we don't miss any events added to the
    // buffer between the call to this method and the beginning of handleEvents()
    eventBufferReader.reset(new EventBufferReader(eventBuffer));
    return std::thread([this]() { handleEvents(); });
}


void ZeroMQOutgoingConnectionBase::handleEvents() {
    while (running) {
        auto event = eventBufferReader->getNextEvent(receiveTimeoutMS * 1000);
        if (event) {
            switch (socket.send(event)) {
                case ZeroMQSocket::Result::timeout:
                    merror(M_NETWORK_MESSAGE_DOMAIN, "Event send timed out");
                    [[clang::fallthrough]];
                    
                case ZeroMQSocket::Result::error:
                    merror(M_NETWORK_MESSAGE_DOMAIN, "Terminating outgoing connection");
                    running = false;
                    return;
                    
                default:
                    break;
            }
        }
    }
    
    // Send termination event
    (void)socket.send(boost::make_shared<Event>(RESERVED_TERMINATION_CODE, Datum()));
}


END_NAMESPACE_MW


























