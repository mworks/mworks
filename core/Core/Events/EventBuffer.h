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


#include "Buffers.h"
#include "Event.h"
#include <boost/thread/mutex.hpp>


BEGIN_NAMESPACE_MW


class EventBuffer : public EventReceiver {
    private:
		boost::mutex bufferLock;
        shared_ptr<Event> headEvent;
		
    public:
		EventBuffer();
    
        void putEvent(shared_ptr<Event> event) override;
        
        shared_ptr<Event> getHeadEvent();
};

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
