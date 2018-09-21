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


extern boost::shared_ptr<EventBuffer> global_outgoing_event_buffer;


void initEventBuffers();


class EventBufferReader {
    
public:
    explicit EventBufferReader(const boost::shared_ptr<EventBuffer> &buffer);
    ~EventBufferReader();
    
    boost::shared_ptr<Event> getNextEvent(MWTime timeoutUS = 0);
    
private:
    boost::shared_ptr<Event> currentEvent;
    
};


END_NAMESPACE_MW


#endif


























