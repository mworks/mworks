/*
 *  EventBuffer.cpp
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 8/12/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */

#include "EventBuffer.h"
using namespace mw;

namespace mw {
	shared_ptr<BufferManager> GlobalBufferManager;
	
	
	void initBuffers() {
		GlobalBufferManager = shared_ptr<BufferManager>(new BufferManager());
	}
}

/**********************************************************************
 *         EventBuffer : public 
 **********************************************************************/

EventBuffer::EventBuffer() {
	// need to prime the buffer with at least one event
	headEvent = shared_ptr<Event>(new Event(M_UNDEFINED_EVENT_CODE,0,Data()));	
}

void EventBuffer::putEvent(shared_ptr<Event> event) {
	boost::mutex::scoped_lock lock(bufferLock);
	headEvent->setNextEvent(event);
	headEvent = event;
}


shared_ptr<Event> EventBuffer::getHeadEvent() {
	boost::mutex::scoped_lock lock(bufferLock);
	return headEvent;
}

void EventBuffer::reset() {
	boost::mutex::scoped_lock lock(bufferLock);
	// need to prime the buffer with at least one event
	headEvent = shared_ptr<Event>(new Event(M_UNDEFINED_EVENT_CODE,0,Data()));	
}

/**********************************************************************
 *      class EventBufferReader : public BufferReader Methods
 **********************************************************************/
EventBufferReader::EventBufferReader(shared_ptr<EventBuffer> buffer) {
	boost::mutex::scoped_lock lock(readerLock);
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

/**********************************************************************
 *                  BufferManager Methods
 **********************************************************************/

BufferManager::BufferManager() {
	main_event_buffer = shared_ptr<EventBuffer>(new EventBuffer());
	incomingBuffer = shared_ptr<EventBuffer>(new EventBuffer());
}

shared_ptr<EventBufferReader> BufferManager::getNewMainBufferReader() {
    shared_ptr<EventBufferReader> reader(new EventBufferReader(main_event_buffer));
    return reader;
}

shared_ptr<EventBufferReader> BufferManager::getNewDisplayBufferReader() {
    shared_ptr<EventBufferReader> reader(new EventBufferReader(main_event_buffer));
    return reader;
}

shared_ptr<EventBufferReader> BufferManager::getNewDiskBufferReader() {
    shared_ptr<EventBufferReader> reader(new EventBufferReader(main_event_buffer));
    return reader;
}

shared_ptr<EventBufferReader> BufferManager::getNewIncomingNetworkBufferReader() {
    shared_ptr<EventBufferReader> reader(new EventBufferReader(incomingBuffer));
    return reader;
}

void BufferManager::putEvent(shared_ptr<Event> newevent) {
    main_event_buffer->putEvent(newevent);
}

void BufferManager::putIncomingNetworkEvent(shared_ptr<Event> newevent) {
    incomingBuffer->putEvent(newevent);
}

void BufferManager::reset() {
	main_event_buffer->reset();
	incomingBuffer->reset();
}

