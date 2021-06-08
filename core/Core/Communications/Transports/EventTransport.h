/*
 *  EventTransport.h
 *  MWorksCore
 *
 *  Created by David Cox on 9/25/08.
 *  Copyright 2008 Harvard University. All rights reserved.
 *
 */

#ifndef  _EVENT_TRANSPORT_H_
#define _EVENT_TRANSPORT_H_


#include <boost/shared_ptr.hpp>
#include "Event.h"


BEGIN_NAMESPACE_MW


// A class that simply moves Event objects from point A
// to point B.  Importantly, point B might be in another
// thread, process, or network location.  "Conduit" objects
// will delegate the details of the communication to Transport
// objects, separating out details of networking, IPC, etc. from
// the details of how state is maintained on either side of the
// conduit (e.g. variables, registries, event buffers, etc.).
// The event transport simply conveys the events without any notion
// of what they might mean


class EventTransport {
   
public:
    enum event_transport_directionality{ bidirectional_event_transport = 0,  // Read and write
                                        read_only_event_transport = 1,      // One way
                                        write_only_event_transport = 2 };

    enum event_transport_type{  symmetric_event_transport = 0,  // Either side can initiate a connection with the other
                               server_event_transport = 1,  // Accepts connections from Client transports
                               client_event_transport = 2 }; // Connects to Server transports
   
protected:
    const event_transport_type type;
    const event_transport_directionality directionality;
    
public:
    EventTransport(event_transport_type type, event_transport_directionality dir) :
        type(type),
        directionality(dir)
    { }
    
    virtual ~EventTransport() {}
    
    event_transport_type getType() const {
        return type;
    }
    
    event_transport_directionality getDirectionality() const {
        return directionality;
    }
    
    // Send event to the other side
    virtual void sendEvent(const boost::shared_ptr<Event> &event) = 0;
    
    // Get an event if one is available; otherwise, return immediately 
    virtual boost::shared_ptr<Event> receiveEventAsynchronous() = 0;
    
};


END_NAMESPACE_MW


#endif
