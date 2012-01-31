/*
 *  SimpleConduit.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 10/1/08.
 *  Copyright 2008 The Rowland Institute at Harvard. All rights reserved.
 *
 */

#include "SimpleConduit.h"
#include "SystemEventFactory.h"
#include "StandardVariables.h"

using namespace mw;

SimpleConduit::SimpleConduit(shared_ptr<EventTransport> _transport, 
                             bool _correct_incoming_timestamps,
                             long _conduit_idle_quantum) :  
    Conduit(_transport), 
    EventCallbackHandler(true),
    correct_incoming_timestamps(correct_incoming_timestamps),
    conduit_idle_quantum_us(_conduit_idle_quantum)
{ }

// Start the conduit working
bool SimpleConduit::initialize(){ 
    
    EventTransport::event_transport_directionality directionality = transport->getDirectionality();
    if(directionality == EventTransport::read_only_event_transport ||
       directionality == EventTransport::bidirectional_event_transport){
        
        // start a read thread to receive events and dispatch callbacks
        read_thread = boost::thread(boost::bind(&SimpleConduit::serviceIncomingEvents, this));
    }
    
    running = true;
    
    // register to receive control events
    // this is needed to be able to handshake regarding time offsets
    registerCallback(RESERVED_SYSTEM_EVENT_CODE, bind(&SimpleConduit::handleSystemEvent, this, _1));
    
    // send a "connected" event to the other side of the conduit
    sendData(SystemEventFactory::connectedEvent());
    
    return true;
}




SimpleConduit::~SimpleConduit(){
    
    int n_wait_attempts = 10;
    int wait_count = 0;
    shared_ptr<Clock> clock = Clock::instance(false);
    
    while(wait_count < n_wait_attempts){
        wait_count++;
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
        
        clock->sleepMS(100);
    }
    
    if(wait_count >= n_wait_attempts){
        std::cerr << "Simple conduit destruction timed out" << std::endl;
    }
}


void SimpleConduit::serviceIncomingEvents(){
    
    shared_ptr<Clock> clock = Clock::instance(false);
    
    bool finished = false;
    
    while(!finished){
        
        bool _stopping = false;
        
        {
            boost::mutex::scoped_lock(stopping_mutex);
            _stopping = stopping;
        }
        
        
        if(_stopping){
            
            {
                boost::mutex::scoped_lock(stopping_mutex);
                stopped = true;
            }
            finished = true;
            break;
        }
        
        
        if(transport == NULL){
            throw SimpleException("Attempt to receive on a NULL event transport");
        }
        
        shared_ptr<Event> incoming_event = transport->receiveEventAsynchronous();
        if(incoming_event == NULL){
            //boost::thread::yield();
            //boost::thread::sleep(boost::posix_time::microsec_clock::local_time() + boost::posix_time::microseconds(conduit_idle_quantum));
            clock->sleepUS(conduit_idle_quantum_us);
            continue;
        }
        
        
        if(correct_incoming_timestamps){
            // if a clock offset has been stored from the other side of the conduit 
            // apply the offset here:
            incoming_event->setTime( incoming_event->getTime() - remote_clock_offset ); 
        }
                
        handleCallbacks(incoming_event);
        
    }
}

// Stop any unfinished business on the conduit; block until 
// everything is done and the object can be safely destroyed.
void SimpleConduit::finalize(){ 
    
    shared_ptr<Clock> clock = Clock::instance(false);
    
    {   // tell the system to stop
        boost::mutex::scoped_lock(stopping_mutex);
        stopping = true;
    }   // release the lock
    
    int n_wait_attempts = 5;
    int wait_count = 0;
    while(wait_count < n_wait_attempts){  // wait for the all clear that the conduit has finished
        
        wait_count++;
        
        bool _stopped = false;
        {
            boost::mutex::scoped_lock(stopping_mutex);
            _stopped = stopped;
        }
        
        if(_stopped){
            break;
        }
        
        clock->sleepMS(100);
    }
    
    
    if(wait_count >= n_wait_attempts){
        std::cerr << "Simple conduit finalization timed out" << std::endl;
    }
    
}


void SimpleConduit::handleSystemEvent(shared_ptr<Event> evt){
    Datum payload_type = evt->getData().getElement(M_SYSTEM_PAYLOAD_TYPE);
    shared_ptr<Clock> clk = Clock::instance();
            
    switch((int)payload_type){
        case M_CLOCK_OFFSET_EVENT:
            if((int)payload_type == M_CLOCK_OFFSET_EVENT){
                remote_clock_offset = clk->getSystemReferenceTime() - (MWTime)evt->getData().getElement(M_SYSTEM_PAYLOAD);
                
            }
            break;
        case M_CONNECTED_EVENT:
            // another conduit is saying hello.  Respond with your local clock offset
            sendData(SystemEventFactory::clockOffsetEvent(clk->getSystemReferenceTime()));
            break;
    }
}


// Send data to the other side.  It is assumed that both sides understand 
// what the event codes mean.
void SimpleConduit::sendData(int code, Datum data){
    shared_ptr<Event> event(new Event(code, data));
    transport->sendEvent(event);
}

void SimpleConduit::sendData(shared_ptr<Event> evt){
    transport->sendEvent(evt);
}


