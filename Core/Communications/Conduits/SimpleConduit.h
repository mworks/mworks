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
#include <hash_map.h>

#include "GenericEventFunctor.h"
#include "SimpleCodecUtilities.h"

#include "EventCallbackHandler.h"

#define DEFAULT_CONDUIT_IDLE_QUANTUM    5000

namespace mw {


class SimpleConduit : public Conduit, public EventCallbackHandler {

protected:
    
    boost::thread read_thread;
    MWTime conduit_idle_quantum_us;
        
public:

    SimpleConduit(shared_ptr<EventTransport> _transport, long _conduit_idle_quantum_us = DEFAULT_CONDUIT_IDLE_QUANTUM);
    
    virtual ~SimpleConduit();

    // Start the conduit working
    virtual bool initialize();
    
    virtual bool isRunning(){
        return !(stopping || stopped);
    }
    
    virtual void serviceIncomingEvents();
    
    // Stop any unfinished business on the conduit; block until 
    // everything is done and the object can be safely destroyed.
    virtual void finalize();
    
    // Send data to the other side.  It is assumed that both sides understand 
    // what the event codes mean.
    virtual void sendData(int code, Datum data);
    virtual void sendData(shared_ptr<Event> evt);
    
    

};

    


}


#endif
