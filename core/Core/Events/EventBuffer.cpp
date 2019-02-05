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


EventBufferReader::~EventBufferReader() {
    //
    // Clear out all remaining events.
    //
    // If we don't do this, and all the events in the buffer are down to a single
    // reference, then destroying currentEvent will recursively destroy its nextEvent,
    // which will recursively destroy *its* nextEvent, and so on.  If there are enough
    // events remaining, this recursive destruction can lead the application to run
    // out of stack space and crash.
    //
    do {
        currentEvent = currentEvent->getNextEvent();
    } while (currentEvent);
}


boost::shared_ptr<Event> EventBufferReader::getNextEvent(MWTime timeoutUS) {
    auto nextEvent = currentEvent->getNextEvent(timeoutUS);
    if (nextEvent) {
        currentEvent = nextEvent;
    }
    return nextEvent;
}


bool EventBufferReader::nextEventExists() const {
    return bool(currentEvent->getNextEvent());
}


END_NAMESPACE_MW


























