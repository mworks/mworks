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
#include "Serialization.h"
#include <string>
#include <boost/interprocess/ipc/message_queue.hpp>
#include "boost/archive/binary_iarchive.hpp"
#include "boost/archive/binary_oarchive.hpp"
#include "boost/serialization/serialization.hpp"
#include <boost/serialization/shared_ptr.hpp> 
#include <boost/static_assert.hpp>
#include <boost/type_traits/make_signed.hpp>

#include <iostream>
#include <sstream>
namespace mw {
using namespace std;


#define QUEUE_PRIORITY 100
#define OUTGOING_SUFFIX "_out"
#define INCOMING_SUFFIX "_in"


class IPCEventTransport : public EventTransport{
    
protected:
    
    static const int MAX_MESSAGE_SIZE = 64000;
    static const int DEFAULT_QUEUE_SIZE = 2000;
    
    string resource_name;
    ostringstream output_stream;
    istringstream input_stream;
    
#if defined(__APPLE__) && defined(__i386__) 
    // On OS X i386, ptrdiff_t is a typedef to int, while size_t is a typedef to unsigned long.  This leads to
    // compilation errors when using the standard boost::interprocess::message_queue, as the implementation seems
    // to depend on ptrdiff_t and size_t differing only in signedness.  The following typedefs define a message_queue
    // variant that replaces ptrdiff_t with make_signed<size_t>::type.  The static assert ensures that the new type
    // is the same size as ptrdiff_t.
    typedef boost::make_signed<std::size_t>::type ptrdiff_type;
    BOOST_STATIC_ASSERT(sizeof(ptrdiff_type) == sizeof(std::ptrdiff_t));
    typedef boost::interprocess::offset_ptr<void, ptrdiff_type> offset_ptr;
    typedef boost::interprocess::message_queue_t<offset_ptr> message_queue;
#else
    typedef boost::interprocess::message_queue message_queue;
#endif
    
    shared_ptr<message_queue> incoming_queue;
    shared_ptr<message_queue> outgoing_queue;
    
public:
    typedef message_queue::size_type message_queue_size_type;
    
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
  
    virtual shared_ptr<Event> deserializeEvent(const char *receive_buffer, message_queue_size_type& received_size);  
    
    
    virtual void flush();
};
}

#endif