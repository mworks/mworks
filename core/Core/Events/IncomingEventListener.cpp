/**
 * IncomingEventListener.cpp
 *
 * History:
 * paul on 1/25/06 - Created.
 *
 * Copyright 2006 MIT. All rights reserved.
 */

#include "IncomingEventListener.h"


BEGIN_NAMESPACE_MW


IncomingEventListener::IncomingEventListener(shared_ptr<EventBuffer> _event_buffer, shared_ptr<EventStreamInterface> hand) :
								EventListener(_event_buffer){
    reader = shared_ptr<EventBufferReader>( new EventBufferReader(_event_buffer) );
    //thread = NULL;
    handler = hand;
    servicing = false;
}

IncomingEventListener::~IncomingEventListener() {
    // TODO should i lock here
    if(thread) {
        thread->cancel();
    }
}

/***************************************************************
 *                      Protected Methods
 **************************************************************/
IncomingEventListener::IncomingEventListener(shared_ptr<EventBuffer> event_buffer) :
							EventListener(event_buffer){}
							
IncomingEventListener::IncomingEventListener(const IncomingEventListener&) {

}

IncomingEventListener& IncomingEventListener::operator=(
                                            const IncomingEventListener&) {
    return *this;
}


END_NAMESPACE_MW
