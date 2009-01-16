/*
 *  EventConduit.h
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 9/25/08.
 *  Copyright 2008 Harvard University. All rights reserved.
 *
 */

#ifndef _CONDUIT_H_
#define _CONDUIT_H_

#include "EventTransport.h"
#include <boost/shared_ptr.hpp>
namespace mw {
using namespace boost;

class Conduit {

protected:
    
    // An object that will handle the actual moving of events
    // from one side to the other.  This object doesn't need to know if the
    // other side is in another thread, process, or network location
    shared_ptr<EventTransport> transport;
    
    bool running;
    
public:
    
    Conduit(shared_ptr<EventTransport> _transport);
    
    virtual ~Conduit(){ }
    
    // Start the conduit working
    virtual bool initialize() = 0;
    
    // Stop any unfinished business on the conduit; block until 
    // everything is done and the object can be safely destroyed.
    virtual void finalize() = 0;
    
};
}
#endif
