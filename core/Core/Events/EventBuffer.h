/*
 *  EventBuffer.h
 *  MWorksCore
 *
 *  Created by David Cox on 8/12/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _EVENTBUFFER_H
#define _EVENTBUFFER_H

#include "Event.h"


BEGIN_NAMESPACE_MW


using EventBuffer = Event::Buffer;


extern shared_ptr<EventBuffer> global_outgoing_event_buffer;
    

class EventBufferReader {

    private:
		shared_ptr<Event>currentEvent;
		boost::mutex readerLock;
		

    public:
    
        explicit EventBufferReader(const shared_ptr<EventBuffer> &buffer);
		bool nextEventExists();
		shared_ptr<Event> getNextEvent();
		bool hasAtLeastNEvents(const unsigned int n);
};
 
    
void initEventBuffers();


END_NAMESPACE_MW


#endif


























