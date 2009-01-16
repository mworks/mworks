/**
* EventListener.cpp
 *
 * History:
 * paul on 1/25/06 - Created.
 *
 * Copyright 2006 MIT. All rights reserved.
 */

#include "EventListener.h"
#include "boost/bind.hpp"
using namespace mw;

// the service function for the thread
static void * readReader(const shared_ptr<EventListener> &el);

EventListener::~EventListener() {
    // TODO should i lock here
    if(thread) {
        thread->cancel();
    }
}

void EventListener::killListener() {
//    fprintf(stderr,"Killing listener..."); fflush(stderr);
	if(thread) {
        thread->cancel();
    }
}

void EventListener::startListener() {
//	
	//	fprintf(stderr,"Starting listener...");fflush(stderr);
    if(thread) { return; } // already started
	// 100 ms interval
	shared_ptr<Scheduler> scheduler = Scheduler::instance();
	shared_ptr<EventListener> this_one = shared_from_this();
	
    thread = scheduler->scheduleUS(FILELINE,
								   0, 
								   100000, 
								   M_REPEAT_INDEFINITELY,
								   boost::bind(readReader, this_one),
								   M_DEFAULT_NETWORK_PRIORITY,
								   M_DEFAULT_NETWORK_WARN_SLOP_US,
								   M_DEFAULT_NETWORK_FAIL_SLOP_US,
								   M_MISSED_EXECUTION_DROP);
}

bool EventListener::service() {
	boost::mutex::scoped_lock lock(listenerLock);
    if(servicing) { 
		return true; 
	}
	
    servicing = true;
    // service all the events received
    while((reader->nextEventExists()) && (handler != NULL)) {
		shared_ptr<Event> event(reader->getNextEvent());
		handler->handleEvent(event);
    }
    servicing = false;
    return true;
}


/***************************************************************
*                      Protected Methods
**************************************************************/
EventListener::EventListener(shared_ptr<BufferManager> manager) {
	buffer_manager = manager;
}

EventListener::EventListener(const EventListener&) {
	
}

EventListener& EventListener::operator=(const EventListener&) {
    return *this;
}

/***************************************************************
*                      Static Methods
**************************************************************/
static void * readReader(const shared_ptr<EventListener> &event_listener) {
    bool rc = event_listener->service();
    if(!rc) {
        event_listener->killListener();
    }
    return NULL;
}
