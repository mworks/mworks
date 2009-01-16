/**
* OutgoingEventListener.cpp
 *
 * History:
 * bkennedy on 9/6/07 - Created.
 *
 * Copyright 2007 MIT. All rights reserved.
 */

#include "OutgoingEventListener.h"
using namespace mw;

OutgoingEventListener::OutgoingEventListener(shared_ptr<BufferManager> manager, shared_ptr<EventHandler> hand) :
								EventListener(manager){
    reader = buffer_manager->getNewDisplayBufferReader();
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

