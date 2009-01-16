/*
 *  DummyEventTransport.h
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 10/1/08.
 *  Copyright 2008 The Rowland Institute at Harvard. All rights reserved.
 *
 */


#ifndef _DUMMY_EVENT_TRANSPORT_H_
#define _DUMMY_EVENT_TRANSPORT_H_

#include "EventTransport.h"
#include "Serialization.h"
#include <string>
#include <boost/interprocess/ipc/message_queue.hpp>
#include "boost/archive/binary_iarchive.hpp"
#include "boost/archive/binary_oarchive.hpp"
#include "boost/serialization/serialization.hpp"
#include <boost/serialization/shared_ptr.hpp> 

#include <iostream>
#include <sstream>
#include <queue>
namespace mw {
using namespace std;

#define MAX_MESSAGE_SIZE    1024
#define DEFAULT_QUEUE_SIZE 2000
#define QUEUE_PRIORITY 100
#define OUTGOING_SUFFIX "_out"
#define INCOMING_SUFFIX "_in"


class DummyEventTransport : public EventTransport{


protected:
    
    typedef shared_ptr< queue< shared_ptr<Event> > >   event_queue_ptr;
    typedef shared_ptr<boost::mutex> mutex_ptr;
    
    static map<string, event_queue_ptr > named_queues;
    static map<string, mutex_ptr > named_queue_locks; 
    
    string resource_name;
    
    event_queue_ptr outgoing_queue;
    event_queue_ptr incoming_queue;
    
    mutex_ptr outgoing_queue_lock;
    mutex_ptr incoming_queue_lock;
    
public:
    
    DummyEventTransport(event_transport_type _type, 
                       event_transport_directionality _dir, 
                       string _resource_name);
                          
    // Send event to the other side
    virtual void sendEvent(shared_ptr<Event> event);
        
    // Get an event from the other side.  Will block if no event is available
    virtual shared_ptr<Event> receiveEvent();
        
    // Get an event if one is available; otherwise, return immediately 
    virtual shared_ptr<Event> receiveEventAsynchronous();
    
    virtual event_queue_ptr getOrCreateQueue(string key){
        if(named_queues.find(key) != named_queues.end()){
            return DummyEventTransport::named_queues[key];
        }
        
        DummyEventTransport::named_queues[key] = event_queue_ptr(new queue< shared_ptr<Event> >());
        
        DummyEventTransport::named_queue_locks[key] = shared_ptr<boost::mutex>(new boost::mutex());
        return DummyEventTransport::named_queues[key];
    }
        
    
};

}
#endif

