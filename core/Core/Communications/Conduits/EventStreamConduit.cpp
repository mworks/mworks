/*
 *  EventStreamConduit.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 10/30/09.
 *  Copyright 2009 Harvard University. All rights reserved.
 *
 *  @discussion  The EventStreamConduit bridges from an interprocess transport
 *  to an existing EventStreamInterface object (prominent examples are the Server and
 *  Client objects which the GUI apps use to access core functionality)
 *
 *
 *  MW-side use case:
 *  - Poll inbound transport for events (requires separate poll thread that is different than
 *    the Event Handler's; this could be events from an IO device, for instance - 
    *** WHAT DO CODES MEAN?  Can the other side send a codec? Would need to have a notion nearside and
    farside codecs and zipper between the two ***)
 *  - Forward inbound events (from the IPC transport) to the event handler's putEvent() (this
      might add the event to MW's event buffers, depending on the handler object).
 *  - Intercepts event forwarding control events (from the transport) and registers appropriate 
 *  callbacks on the event handler for these events; events so registered will sent back over the 
    transport when they occur (it is up to the event handler to dispatch these).  handleEvent calls
    in the handler (e.g. driven by it's polling loop) will then trigger event forwarding.
 *  - Non need to register callbacks directly in this use case; registering callbacks on the event
    handler does everything you'd want to have happen anyways; all registration done by this object
    is behind the scenes.
 
 receiveInboundCodec
 rebuildTranslationCodec
 
 
 SHOULD DO THE BELOW USE CASE AS A SUBCLASS
 pass in a special stub event handler
 additional methods to manipulate the local codec
 EventStreamConduit
 
 *  NON-mw-side use case:
 *  - Poll for inbound events from the other side (e.g. from MW; requires thread)
 *  - allow callbacks to be registered (by name or code) such that inbound events trigger
 *    callback invocation.  The registration of a callback should also send a request to the
 *    other side so that it sends events of this type
 *  - Provide an explicit interface (e.g. sendEvent()) for sending data to the other side
      *** need to have a way to add a name / code pair to a nearside codec, which would get sent
          to the other side; Also needs knoweldge of farside codec to interpret incoming events ***
 *  - Provide an interface to the codec + transparent codec updating
 *  -> Use case can be achieved with a special event handler with generic handle/put event 
 *     methods that are bound to methods on this object.  registerCallback calls then get
       bridged into this event handler.  putEvent should call the handleCallbacks method
       on the handler, invoking registered callbacks.
       
 
 registerCallback -> EventStreamInterface->registerCallback
 
 Special stub EventStreamInterface:
 EventStreamInterface::putEvent -> EventStreamInterface->handleEvent
 EventStreamInterface::handleEvent -> handleCallbacks

 addLocalCode(number, name); -> manipulate EventStreamInterface somehow?  throw if already defined?
 sendCodec()
 */

#include "EventStreamConduit.h"
#include "SystemEventFactory.h"
#include "StandardVariables.h"

#include <functional>

#define CODEC_FROM_STREAM_CALLBACK_TAG "::codec_from_stream_callback"


BEGIN_NAMESPACE_MW


int EventStreamConduit::count = 0;

EventStreamConduit::EventStreamConduit(shared_ptr<EventTransport> _transport, shared_ptr<EventStreamInterface> _stream, MWTime _conduit_idle_quantum_us) : Conduit(_transport){
    event_stream = _stream;
    EventStreamConduit::count++;
    callback_key = (boost::format("MWEventStreamInterface::event_stream_interface_%1%") % EventStreamConduit::count).str();
    conduit_idle_quantum_us = _conduit_idle_quantum_us;
}


bool EventStreamConduit::initialize(){
    
    
    // register a callback on
    event_stream->registerCallback(RESERVED_CODEC_CODE,
                                   boost::bind(&EventStreamConduit::handleCodecFromStream, shared_from_this(), _1),
                                   callback_key + CODEC_FROM_STREAM_CALLBACK_TAG);
        
    // request a codec from the stream side
    event_stream->putEvent(SystemEventFactory::requestCodecControl());
    
    this->registerInternalCallback(RESERVED_SYSTEM_EVENT_CODE, boost::bind(&EventStreamConduit::handleControlEventFromConduit, shared_from_this(), _1));
    
    
    EventTransport::event_transport_directionality directionality = transport->getDirectionality();
    if(directionality == EventTransport::read_only_event_transport ||
       directionality == EventTransport::bidirectional_event_transport){
        
        // start a read thread to receive events and dispatch callbacks
        read_thread = boost::thread(boost::bind(&EventStreamConduit::serviceIncomingEventsFromConduit, this));
    }
    
    running = true;
    
    return true;
  
}


void EventStreamConduit::finalize(){
    // Unregister all callbacks
    {
        boost::mutex::scoped_lock lock(internal_callback_lock);
        unregisterInternalCallbacks();
    }
    {
        boost::mutex::scoped_lock lock(events_to_forward_lock);
        
        event_stream->unregisterCallbacks(callback_key);
        event_stream->unregisterCallbacks(callback_key + CODEC_FROM_STREAM_CALLBACK_TAG);
        
        forward_all_events = false;
        event_codes_to_forward.clear();
        event_names_to_forward.clear();
    }
    
    {   // tell the system to stop
        boost::mutex::scoped_lock lock(stopping_mutex);
        stopping = true;
    }   // release the lock
    
    
    while(true){  // wait for the all clear that the conduit has finished
        
        boost::mutex::scoped_lock lock(stopping_mutex);
        if(stopped){
            break;
        }
        
        boost::thread::sleep(boost::posix_time::microsec_clock::local_time() + boost::posix_time::milliseconds(100));
    }
}


void EventStreamConduit::handleCodec(shared_ptr<Event> event, bool from_conduit_side) {
    // build a map to aid in accessing the codec
    for (auto &item : event->getData().getDict()) {
        auto &key = item.first;
        auto &value = item.second;
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


void EventStreamConduit::handleControlEventFromConduit(shared_ptr<Event> evt){
    
    // mainly interested in M_SET_EVENT_FORWARDING events
    // since these tell us that the other end of this conduit
    // wants us to send or not send it those events
    
    auto &eventData = evt->getData();
    
    // if not event forwarding, pass
    if (eventData.getElement(M_SYSTEM_PAYLOAD_TYPE).getInteger() != M_SET_EVENT_FORWARDING) {
        return;
    }
    
    auto &payload = eventData.getElement(M_SYSTEM_PAYLOAD);
    
    if (payload.hasKey(M_SET_EVENT_FORWARDING_ALL)) {
        
        //
        // Handle request to start/stop forwarding all events
        //
        
        auto shouldForward = payload.getElement(M_SET_EVENT_FORWARDING_ALL).getBool();
        
        boost::mutex::scoped_lock lock(events_to_forward_lock);
        if (forward_all_events != shouldForward) {
            forward_all_events = shouldForward;
            rebuildStreamToConduitForwarding();
        }
        
    } else if (payload.hasKey(M_SET_EVENT_FORWARDING_CODE)) {
        
        //
        // Handle request to start/stop forwarding events with a specific code
        //
        
        int code = payload.getElement(M_SET_EVENT_FORWARDING_CODE).getInteger();
        if (code < 0) {
            return;
        }
        auto shouldForward = payload.getElement(M_SET_EVENT_FORWARDING_STATE).getBool();
        
        boost::mutex::scoped_lock lock(events_to_forward_lock);
        if (shouldForward) {
            if (event_codes_to_forward.insert(code).second && !forward_all_events) {
                startForwardingEvent(code);
            }
        } else {
            if (event_codes_to_forward.erase(code) && !forward_all_events) {
                rebuildStreamToConduitForwarding();
            }
        }
        
    } else if (payload.hasKey(M_SET_EVENT_FORWARDING_NAME)) {
        
        //
        // Handle request to start/stop forwarding events with a specific name
        //
        
        auto &name = payload.getElement(M_SET_EVENT_FORWARDING_NAME).getString();
        if (name.empty()) {
            return;
        }
        auto shouldForward = payload.getElement(M_SET_EVENT_FORWARDING_STATE).getBool();
        
        boost::mutex::scoped_lock lock(events_to_forward_lock);
        if (shouldForward) {
            if (event_names_to_forward.insert(name).second && !forward_all_events) {
                startForwardingEvent(name);
            }
        } else {
            if (event_names_to_forward.erase(name) && !forward_all_events) {
                rebuildStreamToConduitForwarding();
            }
        }
        
    }
}


void EventStreamConduit::rebuildStreamToConduitForwarding(){
    // unregister any callbacks previously registered by this object
    event_stream->unregisterCallbacks(callback_key);
    
    if (forward_all_events) {
        event_stream->registerCallback(boost::bind(&EventStreamConduit::sendData, shared_from_this(), _1), callback_key);
        // Since we're forwarding all events, there's no need to register individual callbacks for specific codes
        // or names.  We're done.
        return;
    }
    
    // for each event code in event_codes_to_forward, register a callback
    // using the event stream interface
    for (auto code : event_codes_to_forward) {
        startForwardingEvent(code);
    }
    
    // for each event name in event_names_to_forward, register a callback
    // using the event stream interface
    for (auto &name : event_names_to_forward) {
        startForwardingEvent(name);
    }
}


void EventStreamConduit::startForwardingEvent(int code) {
    if (code >= 0) {
        event_stream->registerCallback(code, boost::bind(&EventStreamConduit::sendData, shared_from_this(), _1), callback_key);
    }
}


void EventStreamConduit::startForwardingEvent(const std::string &tag_to_forward) {
    if(!tag_to_forward.empty()){
        // if the tag is listed
        if(stream_side_reverse_codec.find(tag_to_forward) != stream_side_reverse_codec.end()){
            int code = stream_side_reverse_codec[tag_to_forward];
            startForwardingEvent(code);
        }
    }
}


void EventStreamConduit::serviceIncomingEventsFromConduit(){
    
    shared_ptr<Clock> clock = Clock::instance(false);
    
    while(1){
        
        {
            boost::mutex::scoped_lock lock(stopping_mutex);
            if(stopping){
                stopped = true;
                break;
            }
        }
        
        
        if(transport == NULL){
            throw SimpleException("Attempt to receive on a NULL event transport");
        }
        
        shared_ptr<Event> incoming_event = transport->receiveEventAsynchronous();
        if(incoming_event == NULL){
            //boost::thread::yield();
            clock->sleepUS(conduit_idle_quantum_us);
            //boost::thread::sleep(boost::posix_time::microsec_clock::local_time() + boost::posix_time::microseconds(500));
            continue;
        }
        int event_code = incoming_event->getEventCode();
        
        {   // scope for locking
            boost::mutex::scoped_lock lock(internal_callback_lock);
            if(internal_callbacks.find(event_code) != internal_callbacks.end()){
                internal_callbacks[event_code](incoming_event);
            }
        }
        
        event_stream->putEvent(incoming_event);
        
    }
}


END_NAMESPACE_MW
