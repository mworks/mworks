/*
 *  SimpleConduit.h
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 10/1/08.
 *  Copyright 2008 The Rowland Institute at Harvard. All rights reserved.
 *
 */

// A thin wrapper around an event transport.  It is assumed here that both
// sides of the communication already know what the event codes mean; no effort
// will be made to transmit codecs, translate codes, etc.
// A prime use-case for this class is in the create of networked or 
// separate-process IO devices.  This case, the event codes should be enums
// (or similar) that are compiled-in on both sides (these events will be 
// "private" between the two sides of the conduit, and will not be put direclty 
// into a file or other event stream).


#ifndef _SIMPLE_CONDUIT_H_
#define _SIMPLE_CONDUIT_H_


#include "Conduit.h"
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "GenericEventFunctor.h"
namespace mw {

typedef boost::function< void (shared_ptr<Event>) >  event_callback;

class SimpleConduit : public Conduit {

protected:

    map<int, event_callback> callbacks;
    boost::thread read_thread;
    
    boost::mutex stopping_mutex;
    bool stopping;
    bool stopped;
    
    MonkeyWorksTime timeout_ms;
    
public:

    SimpleConduit(shared_ptr<EventTransport> _transport);
    
    virtual ~SimpleConduit();

    // Start the conduit working
    virtual bool initialize();
    
    virtual void serviceIncomingEvents();
    
    // Stop any unfinished business on the conduit; block until 
    // everything is done and the object can be safely destroyed.
    virtual void finalize();
    
    // Register an event callback
    virtual void registerCallback(int event_code, event_callback functor);

    // Send data to the other side.  It is assumed that both sides understand 
    // what the event codes mean.
    virtual void sendData(int code, Data data);
    
    
};
}

#endif
