/*
 *  EventStreamInterfaceConduit.h
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 10/30/09.
 *  Copyright 2009 Harvard University. All rights reserved.
 *
 */
#ifndef EVENT_HANDLER_CONDUIT_H_
#define EVENT_HANDLER_CONDUIT_H_

#include "Conduit.h"
#include "Event.h"
#include "SimpleConduit.h"
#include "EventTransport.h"
#include "EventStreamInterface.h"
#include "boost/enable_shared_from_this.hpp"

#define EVENT_HANDLER_CONDUIT_CALLBACK_KEY "EVENT_HANDLER_CONDUIT_CALLBACK_KEY"

namespace mw {
using namespace boost;
using namespace std;

    
class EventStreamInterfaceConduit : public Conduit, public enable_shared_from_this<EventStreamInterfaceConduit> {

protected:
    
    shared_ptr<EventStreamInterface> event_handler;
    map<string, int> reverse_codec_map;
    boost::mutex events_to_forward_lock;
    list<string> events_to_forward;
    list<int> codes_to_forward;
    
public:
    
    EventStreamInterfaceConduit(shared_ptr<EventTransport> _transport, shared_ptr<EventStreamInterface> _handler);
    
    virtual ~EventStreamInterfaceConduit(){  }
    
    virtual bool initialize();
    
    // bridge requests for callbacks through to the event handler
    // also, send a event forwarding request over the conduit to the other side
    //void registerCallback(){ }
    
    
    // a thread/loop to poll for events on the event transport and pass them into the
    // event handler for callback dispatch
    virtual void serviceIncomingEvents();
                          
    // Rebuild the internal representation of the codec, and launch a rebuild
    // of the callback map
    void handleCodec(shared_ptr<Event> event){
        Datum codec = event->getData();
        vector<Datum> keys = codec.getKeys();
        
        // build a map to aid in accessing the codec
        for(int i=0; i < keys.size(); i++){
            reverse_codec_map[codec.getElement(keys[i])] = keys[i];
        }
        
        
        boost::mutex::scoped_lock lock(events_to_forward_lock);
        // erase the codes to forward structure so we can rebuild it
        codes_to_forward.clear();
        
        // Rebuild codes_to_forward according to the new codec
        rebuildCodesToForward();
        
        sendData(RESERVED_CODEC_CODE, codec);
        
    }
    
    // Rebuild the code -> callback mapping, taking into account any new information
    // from an inbound codec
    void rebuildCodesToForward();    

    
    // Handle incoming requests for change in event forwarding (e.g. if the
    // other side wants to receive a particular kind of event)
    void handleControlEvent(shared_ptr<Event> evt){
    
        // mainly interested in M_SET_EVENT_FORWARDING events
        // since these tell us that the other end of this conduit
        // wants us to send or not send it those events
        
        Datum payload_type = evt->getData().getElement(M_SYSTEM_PAYLOAD_TYPE);
        
        
        // if not event forwarding, pass
        if((int)payload_type != M_SET_EVENT_FORWARDING){
            return;
        }
        
        Datum payload(evt->getData().getElement(M_SYSTEM_PAYLOAD));
        
        Datum event_id_datum(payload.getElement(M_SET_EVENT_FORWARDING_NAME));
        Datum state_datum(payload.getElement(M_SET_EVENT_FORWARDING_STATE));
        
        string event_name;
        if(event_id_datum.isString()){
            event_name = event_id_datum.getString();
        } else {
            throw SimpleException("Unknown data type for event forwarding request");
        }
        
        
        boost::mutex::scoped_lock lock(events_to_forward_lock);
        list<string>::iterator event_iterator = find_if(events_to_forward.begin(), events_to_forward.end(), bind2nd(equal_to<string>(),event_name));
        
        if(state_datum.getBool()){
            if(event_iterator == events_to_forward.end()){
                events_to_forward.push_back(event_name);
            }
        } else {
            
            if(event_iterator != events_to_forward.end()){
                events_to_forward.erase(event_iterator);
            }
        }
        
        rebuildCodesToForward();
    }
    
    
};
    
}



#endif