/*
 *  SimpleConduit.cpp
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 10/1/08.
 *  Copyright 2008 The Rowland Institute at Harvard. All rights reserved.
 *
 */

#include "SimpleConduit.h"
#include "ControlEventFactory.h"
#include "StandardVariables.h"

using namespace mw;

SimpleConduit::SimpleConduit(shared_ptr<EventTransport> _transport) :  Conduit(_transport){ }

// Start the conduit working
bool SimpleConduit::initialize(){ 
    
    EventTransport::event_transport_directionality directionality = transport->getDirectionality();
    if(directionality == EventTransport::read_only_event_transport ||
       directionality == EventTransport::bidirectional_event_transport){
        
        // start a read thread to receive events and dispatch callbacks
        read_thread = boost::thread(boost::bind(&SimpleConduit::serviceIncomingEvents, this));
    }
    
    running = true;
    
    // register to receive codec events
    
    return true;
}

SimpleConduit::~SimpleConduit(){
    
    while(true){
        bool is_stopping, is_stopped;
        {
            boost::mutex::scoped_lock(stopping_mutex);
            is_stopping = stopping;
            is_stopped = stopped;
        }
        
        if(!(is_stopping || stopped)){
            finalize();
        }
        
        if(stopped){
            break;
        }
        
        boost::thread::sleep(boost::posix_time::microsec_clock::local_time() + boost::posix_time::milliseconds(1));
    }
}


void SimpleConduit::serviceIncomingEvents(){
    
    while(1){
        
        {
            boost::mutex::scoped_lock(stopping_mutex);
            if(stopping){
                stopped = true;
                break;
            }
        }
        
        
        if(transport == NULL){
            throw SimpleException("Attempt to receive on a NULL event transport");
        }
        
        shared_ptr<Event> incoming_event = transport->receiveEventAsynchronous();
        if(incoming_event == NULL){
            boost::thread::yield();
            continue;
        }
        int event_code = incoming_event->getEventCode();
        if(callbacks.find(event_code) != callbacks.end()){
            callbacks[event_code](incoming_event);
        }
        
        
    }
}

// Stop any unfinished business on the conduit; block until 
// everything is done and the object can be safely destroyed.
void SimpleConduit::finalize(){ 
    

    {   // tell the system to stop
        boost::mutex::scoped_lock(stopping_mutex);
        stopping = true;
    }   // release the lock
    
    
    while(true){  // wait for the all clear that the conduit has finished
        
        boost::mutex::scoped_lock(stopping_mutex);
        if(stopped){
            break;
        }
        
        boost::thread::sleep(boost::posix_time::microsec_clock::local_time() + boost::posix_time::milliseconds(100));
    }
    
    
}

// Register an event callback
void SimpleConduit::registerCallback(int event_code, event_callback functor){
    if(running){
        throw SimpleException("Cannot register a callback on an already-running conduit");
    }
    
    callbacks[event_code] = functor;
}


void SimpleConduit::registerCallback(string event_name, event_callback functor){
    int event_code = -1;
    
    // lookup event code
    
    registerCallback(event_code, functor);
    
    sendData(ControlEventFactory::setEventForwardingControl(event_name, true));
}


// Send data to the other side.  It is assumed that both sides understand 
// what the event codes mean.
void SimpleConduit::sendData(int code, Datum data){
    //fprintf(stderr, "sending event");fflush(stderr);
    shared_ptr<Event> event(new Event(code, data));
    transport->sendEvent(event);
}

void SimpleConduit::sendData(shared_ptr<Event> evt){
    transport->sendEvent(evt);
}


