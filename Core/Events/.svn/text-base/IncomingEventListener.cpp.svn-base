/**
 * IncomingEventListener.cpp
 *
 * History:
 * paul on 1/25/06 - Created.
 *
 * Copyright 2006 MIT. All rights reserved.
 */

#include "IncomingEventListener.h"

using namespace mw;

IncomingEventListener::IncomingEventListener(shared_ptr<BufferManager> manager, shared_ptr<EventHandler> hand) :
								EventListener(manager){
    reader = buffer_manager->getNewIncomingNetworkBufferReader();
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
IncomingEventListener::IncomingEventListener(shared_ptr<BufferManager> manager) :
							EventListener(manager){}
							
IncomingEventListener::IncomingEventListener(const IncomingEventListener&) {

}

IncomingEventListener& IncomingEventListener::operator=(
                                            const IncomingEventListener&) {
    return *this;
}
