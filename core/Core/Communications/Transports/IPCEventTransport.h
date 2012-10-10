/*
 *  IPCEventTransport.h
 *  MWorksCore
 *
 *  Created by David Cox on 9/25/08.
 *  Copyright 2008 Harvard University. All rights reserved.
 *
 */

#ifndef _IPC_EVENT_TRANSPORT_H_
#define _IPC_EVENT_TRANSPORT_H_

#include "EventTransport.h"
#include "IPCUtilities.h"

#include <string>


BEGIN_NAMESPACE_MW


class IPCEventTransport : public EventTransport{
    
protected:
    
    static const int MAX_MESSAGE_SIZE = 262144;  // 256kB
    static const int DEFAULT_QUEUE_SIZE = 500;
    static const unsigned int QUEUE_PRIORITY = 100;
    
    static const std::string OUTGOING_SUFFIX;
    static const std::string INCOMING_SUFFIX;
    
    string resource_name;
    
    typedef message_queue_32_64::message_queue message_queue;
    typedef message_queue_32_64::oarchive oarchive;
    typedef message_queue_32_64::iarchive iarchive;
    
    shared_ptr<message_queue> incoming_queue;
    shared_ptr<message_queue> outgoing_queue;
    
    char receive_buffer[MAX_MESSAGE_SIZE];
    
public:
    typedef message_queue::size_type message_queue_size_type;
    
    IPCEventTransport(event_transport_type _type, 
                      event_transport_directionality _dir, 
                      std::string _resource_name);
                          
    // Send event to the other side
    virtual void sendEvent(shared_ptr<Event> event);
        
    // Get an event from the other side.  Will block if no event is available
    virtual shared_ptr<Event> receiveEvent();
        
    // Get an event if one is available; otherwise, return immediately 
    virtual shared_ptr<Event> receiveEventAsynchronous();
  
    virtual shared_ptr<Event> deserializeEvent(message_queue_size_type& received_size);  
    
    virtual void flush();
};


END_NAMESPACE_MW


#endif