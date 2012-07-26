/**
 * EventStreamInterface.cpp
 *
 * History:
 * paul on 1/27/06 - Created.
 *
 * Copyright 2006 MIT. All rights reserved.
 */

#include "EventStreamInterface.h"
using namespace mw;

using namespace std;


EventStreamInterface::EventStreamInterface(MessageDomain _dom, bool locking) : EventCallbackHandler(locking){
    message_domain = _dom;
}

EventStreamInterface::~EventStreamInterface() { }

