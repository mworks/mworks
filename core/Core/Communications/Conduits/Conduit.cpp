/*
 *  Conduit.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 9/25/08.
 *  Copyright 2008 Harvard University. All rights reserved.
 *
 */

#include "Conduit.h"


BEGIN_NAMESPACE_MW


Conduit::Conduit(shared_ptr<EventTransport> _transport){
    transport = _transport;
    running = false;
    stopping = false;
    stopped = false;
}



// Send data to the other side.  It is assumed that both sides understand 
// what the event codes mean.
void Conduit::sendData(int code, Datum data){
    //fprintf(stderr, "sending event");fflush(stderr);
    shared_ptr<Event> event(new Event(code, data));
    sendData(event);
}

void Conduit::sendData(shared_ptr<Event> evt){
    transport->sendEvent(evt);
}


END_NAMESPACE_MW
