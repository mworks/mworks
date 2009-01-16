/*
 *  IPCEventTransport.h
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 9/25/08.
 *  Copyright 2008 Harvard University. All rights reserved.
 *
 */

#ifndef _IPC_EVENT_TRANSPORT_H_
#define _IPC_EVENT_TRANSPORT_H_

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
namespace mw {
using namespace std;

#define MAX_MESSAGE_SIZE    1024
#define DEFAULT_QUEUE_SIZE 2000
#define QUEUE_PRIORITY 100
#define OUTGOING_SUFFIX "_out"
#define INCOMING_SUFFIX "_in"


class IPCEventTransport : public EventTransport{
    
protected:
    
    string resource_name;
    ostringstream output_stream;
    istringstream input_stream;
    
    shared_ptr<interprocess::message_queue> incoming_queue;
    shared_ptr<interprocess::message_queue> outgoing_queue;
    
public:
    
    IPCEventTransport(event_transport_type _type, 
                       event_transport_directionality _dir, 
                       string _resource_name);
                          
    // Send event to the other side
    virtual void sendEvent(shared_ptr<Event> event);
        
    // Get an event from the other side.  Will block if no event is available
    virtual shared_ptr<Event> receiveEvent();
        
    // Get an event if one is available; otherwise, return immediately 
    virtual shared_ptr<Event> receiveEventAsynchronous();
    
    virtual shared_ptr<Event> receiveEventNoLock();
  
    virtual shared_ptr<Event> deserializeEvent(const char *receive_buffer, size_t& received_size);  
};
}

#endif