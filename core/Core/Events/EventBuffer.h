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
    
    bool nextEventExists() const;
    boost::shared_ptr<Event> getNextEvent(MWTime timeoutUS = 0);
    
private:
    using lock_guard = std::lock_guard<std::mutex>;
    
    boost::shared_ptr<Event> currentEvent;
    mutable lock_guard::mutex_type mutex;
    
};


END_NAMESPACE_MW


#endif


























