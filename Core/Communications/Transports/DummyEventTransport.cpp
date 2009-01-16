/*
 *  DummyTransport.cpp
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 10/1/08.
 *  Copyright 2008 The Rowland Institute at Harvard. All rights reserved.
 *
 */

#include "DummyEventTransport.h"
using namespace mw;

map<string, DummyEventTransport::event_queue_ptr> DummyEventTransport::named_queues;
map<string, DummyEventTransport::mutex_ptr> DummyEventTransport::named_queue_locks;

DummyEventTransport::DummyEventTransport(event_transport_type _type, 
                       event_transport_directionality _dir, 
                       string _resource_name) : EventTransport(_type, _dir) {


    resource_name = _resource_name;
    string resource_name_outgoing = resource_name + OUTGOING_SUFFIX;
    string resource_name_incoming = resource_name + INCOMING_SUFFIX;
    
    
    if(type == server_event_transport){
        outgoing_queue = getOrCreateQueue(resource_name_outgoing);
        outgoing_queue_lock = named_queue_locks[resource_name_outgoing];
        incoming_queue = getOrCreateQueue(resource_name_incoming);
        incoming_queue_lock = named_queue_locks[resource_name_incoming];
    }

    if(type == client_event_transport){
        outgoing_queue = getOrCreateQueue(resource_name_incoming);
        outgoing_queue_lock = named_queue_locks[resource_name_incoming];
        incoming_queue = getOrCreateQueue(resource_name_outgoing);
        incoming_queue_lock = named_queue_locks[resource_name_outgoing];
    }
    
}

                          
// Send event to the other side
void DummyEventTransport::sendEvent(shared_ptr<Event> event){

    boost::mutex::scoped_lock locker(*outgoing_queue_lock);
    outgoing_queue->push(event);
}
        
// Get an event from the other side.  Will block if no event is available
shared_ptr<Event> DummyEventTransport::receiveEvent(){
    return shared_ptr<Event>();
}
        
// Get an event if one is available; otherwise, return immediately 
shared_ptr<Event> DummyEventTransport::receiveEventAsynchronous(){
    boost::mutex::scoped_lock locker(*outgoing_queue_lock);
    
    if(incoming_queue->size() == 0){
        return shared_ptr<Event>();
    }
    
    shared_ptr<Event> event = incoming_queue->front();
    incoming_queue->pop();
    
    return event;
}
   