/**
* EventListener.cpp
 *
 * History:
 * paul on 1/25/06 - Created.
 *
 * Copyright 2006 MIT. All rights reserved.
 */

#include "EventListener.h"


BEGIN_NAMESPACE_MW


EventListener::EventListener(const boost::shared_ptr<EventBuffer> &event_buffer,
                             const boost::shared_ptr<EventStreamInterface> &handler) :
    reader(event_buffer),
    handler(handler),
    servicing(false)
{ }


EventListener::~EventListener() {
    killListener();
}


void EventListener::startListener() {
    if (!thread.joinable()) {
        servicing = true;
        thread = std::thread([this]() { service(); });
    }
}


void EventListener::killListener() {
    if (thread.joinable()) {
        servicing = false;
        thread.join();
    }
}


void EventListener::service() {
    while (servicing) {
        auto event = reader.getNextEvent(500000);  // 500ms timeout
        if (event) {
            handler->handleEvent(event);
        }
    }
}


END_NAMESPACE_MW

























