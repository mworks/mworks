/*
 *  EventBuffer.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 8/12/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */

#include "EventBuffer.h"

#include "Utilities.h"


BEGIN_NAMESPACE_MW


boost::shared_ptr<EventBuffer> global_outgoing_event_buffer;


void initEventBuffers() {
    global_outgoing_event_buffer = boost::make_shared<EventBuffer>();
}


EventBufferReader::EventBufferReader(const boost::shared_ptr<EventBuffer> &buffer) :
    currentEvent(buffer->getHeadEvent())
{ }


boost::shared_ptr<Event> EventBufferReader::getNextEvent() {
    lock_guard lock(mutex);
    
    auto nextEvent = currentEvent->getNextEvent();
    if (nextEvent) {
        currentEvent = nextEvent;
    }
    
    return nextEvent;
}


bool EventBufferReader::nextEventExists() const {
    lock_guard lock(mutex);
    return bool(currentEvent->getNextEvent());
}


END_NAMESPACE_MW


























