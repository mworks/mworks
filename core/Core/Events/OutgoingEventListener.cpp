/**
* OutgoingEventListener.cpp
 *
 * History:
 * bkennedy on 9/6/07 - Created.
 *
 * Copyright 2007 MIT. All rights reserved.
 */

#include "OutgoingEventListener.h"


BEGIN_NAMESPACE_MW


OutgoingEventListener::OutgoingEventListener(shared_ptr<EventBuffer> _event_buffer, shared_ptr<EventStreamInterface> hand) :
								EventListener(_event_buffer){
    reader = shared_ptr<EventBufferReader>( new EventBufferReader(_event_buffer) );
    //thread = NULL;
    handler = hand;
    servicing = false;
}

OutgoingEventListener::~OutgoingEventListener() {
    // TODO should i lock here
    if(thread) {
        thread->cancel();
    }
}

/***************************************************************
*                      Protected Methods
**************************************************************/
OutgoingEventListener::OutgoingEventListener() {}

OutgoingEventListener::OutgoingEventListener(const OutgoingEventListener&) {
	
}

OutgoingEventListener& OutgoingEventListener::operator=(
														  const OutgoingEventListener&) {
    return *this;
}


END_NAMESPACE_MW
