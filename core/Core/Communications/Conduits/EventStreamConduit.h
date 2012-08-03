/*
 *  EventStreamConduit.h
 *  MWorksCore
 *
 *  Created by David Cox on 10/30/09.
 *  Copyright 2009 Harvard University. All rights reserved.
 *
 *
 *  An object that endeavors to seemlessly bridge between an 
 *  interprocess conduit (e.g. shared memory with another process)
 *  on one side, and a MW event stream on the other
 *
 *  To achieve this, the object must be responsible for keeping track of
 *  event codecs on both sides, with potentially different mappings 
 *  between event names and codes, and it must be able to respond to
 *  event forwarding requests from the conduit-side to determine which
 *  events are forwarded from the stream-side to the conduit-side
 */
#ifndef EVENT_STREAM_CONDUIT_H_
#define EVENT_STREAM_CONDUIT_H_

#include "Conduit.h"
#include "Event.h"
#include "SimpleConduit.h"
#include "EventTransport.h"
#include "EventStreamInterface.h"
#include "SystemEventFactory.h"
#include "boost/enable_shared_from_this.hpp"


namespace mw {
using namespace boost;
using namespace std;

#define EVENT_STREAM_CONDUIT_DEFAULT_IDLE_QUANTUM   500
    
class EventStreamConduit : public Conduit, public enable_shared_from_this<EventStreamConduit> {

protected:
    
    // An interface to an MW event stream (e.g. a client or server instance)
    shared_ptr<EventStreamInterface> event_stream;
    
    // the latest codec received from the conduit side
    map<string, int> conduit_side_reverse_codec;
    map<int, string> conduit_side_codec;
    
    // the latest codec from the event stream side
    map<string, int> stream_side_reverse_codec;
    map<int, string> stream_side_codec;
    
    // A list of event names that the conduit side has requested be forwarded to it
    boost::mutex events_to_forward_lock;
    list<string> events_to_forward;
    
    // A unique key that we can use to unregister/re-register callbacks made by 
    // this object
    string callback_key;
    
    // An internal count of the number of conduits of this type
    static int count;
    
    // A thread for polling the conduit side for incoming events
    boost::thread read_thread;
    
    // Internal callbacks, to respond to incoming events on the conduit
    map<int, EventCallback> internal_callbacks;
    boost::mutex internal_callback_lock;
    
    MWTime conduit_idle_quantum_us;
    
    // these will be called from callbacks, so it is not necessary to lock them
    void registerInternalCallback(int event_code, EventCallback functor){
        //boost::mutex::scoped_lock lock(internal_callback_lock);
        internal_callbacks[event_code] = functor;
    }
    
    void unregisterInternalCallbacks(){
        //boost::mutex::scoped_lock lock(internal_callback_lock);
        internal_callbacks.clear();
    }
    
    // In response to new information (assume conduit_side_codec and 
    // conduit_side_reverse_codec are valid):
    // 1) Unregister old stream-side callbacks
    // 2) Re-register new stream-side callbacks using the stream-side codec 
    void rebuildStreamToConduitForwarding();    
    
    void startForwardingEvent(const std::string &name);
    
public:
    
    EventStreamConduit(shared_ptr<EventTransport> _transport, shared_ptr<EventStreamInterface> _handler, MWTime _conduit_idle_quantum_us = EVENT_STREAM_CONDUIT_DEFAULT_IDLE_QUANTUM);
    virtual ~EventStreamConduit(){  }
    
    // Necessary methods for all conduits
    virtual bool initialize();
    virtual void finalize();
    
    
    // Service events coming in from the other side of the conduit
    // This includes:
    // 1) Receiving conduit-side codecs, and updating appropriately
    // 2) Receiving event-forwarding requests, and updating stream-side
    //    callbacks and translation codecs appropriately
    // 3) Forwarding conduit-side events that have meaning (translatable
    //    codes) to the stream-side
    virtual void serviceIncomingEventsFromConduit();
    
    
    // Rebuild the internal representation of the codec, and launch a rebuild
    // of the callback map
    void handleCodec(shared_ptr<Event> event, bool from_conduit_side){
        Datum codec = event->getData();
        vector<Datum> keys = codec.getKeys();
        
        // build a map to aid in accessing the codec
        for(int i=0; i < keys.size(); i++){
            
            Datum key = keys[i];
            Datum value = codec.getElement(keys[i]);
            string tagname;
            if(value.getDataType() == M_STRING){
                tagname = value.getString();
            } else if(value.getDataType() == M_DICTIONARY){
                tagname = string(value.getElement("tagname"));
            } else {
                mwarning(M_SYSTEM_MESSAGE_DOMAIN, "Invalid values in codec received by EventStreamConduit");
                continue;
            }
            
            if(from_conduit_side){
                conduit_side_codec[(int)key] = tagname;
                conduit_side_reverse_codec[tagname] = (int)key;
            } else {
                stream_side_codec[(int)key] = tagname;
                stream_side_reverse_codec[tagname] = (int)key;
            }
        }
        
        if(!from_conduit_side){
            // forward the codec over the conduit
            //std::cerr << "Forwarding codec over conduit" << std::endl;
            sendData(event);
        }
        
        // Rebuild codes_to_forward according to the new codec
        boost::mutex::scoped_lock lock(events_to_forward_lock);
        rebuildStreamToConduitForwarding();
        
    }
    
    void handleCodecFromConduit(shared_ptr<Event> event){
        return handleCodec(event, true);
    }
    
    void handleCodecFromStream(shared_ptr<Event> event){
        return handleCodec(event, false);
    }

    // Handle incoming requests for change in event forwarding (e.g. if the
    // other side wants to receive a particular kind of event)
    void handleControlEventFromConduit(shared_ptr<Event> evt);
    
    
};
    
}



#endif