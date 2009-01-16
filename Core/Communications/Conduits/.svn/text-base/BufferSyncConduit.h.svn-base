/*
 *  BufferSyncConduit.h
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 9/30/08.
 *  Copyright 2008 The Rowland Institute at Harvard. All rights reserved.
 *
 */

// The Buffer Synchronization Conduit handles moving events coming off of 
// an EventTransport into a live system's event buffers.  This kind of 
// Conduit could be used, for instance, to synchronize the state of two
// "full" systems, by ensure that all events happening on one side
// appear on the other side, as if they had happened locally.

#ifndef _BUFFER_SYNC_CONDUIT_H_
#define _BUFFER_SYNC_CONDUIT_H_


#include "Conduit.h"
#include "VariableRegistry.h"
#include "EventFactory.h"
namespace mw {
class BufferSyncConduit : public Conduit {
    
protected:
    shared_ptr<BufferManager> buffer_manager;
    shared_ptr<VariableRegistry> registry;

public:
    BufferSyncConduit(shared_ptr<EventTransport> _transport,
                       shared_ptr<VariableRegistry> _registry) : Conduit(_transport){
        registry = _registry;
        buffer_manager = registry->getBufferManager();
        
        // send the variable codec, as a matter of course
        transport->sendEvent(EventFactory::codecPackage());
    }
    
    virtual ~BufferSyncConduit(){ }
    
    virtual shared_ptr<BufferManager> getBufferManager(){
        return buffer_manager;
    }
    
    virtual shared_ptr<VariableRegistry> getVariableRegistry(){
        return registry;
    }
    
    // Start the conduit working
    virtual bool initialize(){
    
        // start read and write threads
        EventTransport::event_transport_directionality directionality = transport->getDirectionality();
        
        if(directionality == EventTransport::read_only_event_transport ||
           directionality == EventTransport::bidirectional_event_transport){
            
            // start read thread
        }
        
        if(directionality == EventTransport::write_only_event_transport ||
           directionality == EventTransport::bidirectional_event_transport){
            
            // start write thread
        }
		return true;
    }
    
    // Stop any unfinished business on the conduit; block until 
    // everything is done and the object can be safely destroyed.
    virtual void finalize(){
    
    }

};
}

#endif
