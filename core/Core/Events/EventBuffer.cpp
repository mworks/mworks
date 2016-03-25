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


shared_ptr<EventBuffer> global_outgoing_event_buffer;


void initEventBuffers() {
    global_outgoing_event_buffer = shared_ptr<EventBuffer>(new EventBuffer());
}


/**********************************************************************
 *      class EventBufferReader : public BufferReader Methods
 **********************************************************************/

EventBufferReader::EventBufferReader(const shared_ptr<EventBuffer> &buffer) {
	currentEvent = buffer->getHeadEvent();
}

shared_ptr<Event> EventBufferReader::getNextEvent() {
	boost::mutex::scoped_lock lock(readerLock);
	
	if(currentEvent->getNextEvent() != 0) {
		currentEvent = currentEvent->getNextEvent();
		return currentEvent;
	} else {
		merror(M_SYSTEM_MESSAGE_DOMAIN, "trying to get an event that is not available");
		return shared_ptr<Event>();
	}
}

bool EventBufferReader::nextEventExists() {
	boost::mutex::scoped_lock lock(readerLock);
	return (currentEvent->getNextEvent() != 0);
}

bool EventBufferReader::hasAtLeastNEvents(const unsigned int n) {
	shared_ptr<Event> eventCounter = currentEvent;
	for(unsigned int i=0; i<n; ++i) {
		eventCounter = eventCounter->getNextEvent();
		if(eventCounter == 0) {
			return false;
		}
	}
	
	return true;
}


END_NAMESPACE_MW
