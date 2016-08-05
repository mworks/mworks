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


ZeroMQConnection::ZeroMQConnection(ZeroMQSocket &socket, const boost::shared_ptr<EventBuffer> &eventBuffer) :
    socket(socket),
    eventBuffer(eventBuffer),
    running(false)
{ }


ZeroMQConnection::~ZeroMQConnection() {
    stop();
}


void ZeroMQConnection::start() {
    if (!eventHandlerThread.joinable()) {
        prepare();
        running = true;
        eventHandlerThread = std::thread([this]() { handleEvents(); });
    }
}


void ZeroMQConnection::stop() {
    if (eventHandlerThread.joinable()) {
        running = false;
        eventHandlerThread.join();
    }
}


void ZeroMQIncomingConnection::handleEvents() {
    if (!socket.setOption(ZMQ_RCVTIMEO, receiveTimeoutMS)) {
        return;
    }
    
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


void ZeroMQOutgoingConnection::prepare() {
    // Create the buffer reader here, so that we don't miss any events added to the
    // buffer between the call to start() and the beginning of handleEvents()
    eventBufferReader.reset(new EventBufferReader(eventBuffer));
}


void ZeroMQOutgoingConnection::handleEvents() {
    if (!socket.setOption(ZMQ_SNDTIMEO, sendTimeoutMS)) {
        return;
    }
    
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


























