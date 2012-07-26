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

using namespace mw;


int EventStreamConduit::count = 0;

EventStreamConduit::EventStreamConduit(shared_ptr<EventTransport> _transport, shared_ptr<EventStreamInterface> _stream, MWTime _conduit_idle_quantum_us) : Conduit(_transport){
    event_stream = _stream;
    EventStreamConduit::count++;
    callback_key = (boost::format("MWEventStreamInterface::event_stream_interface_%1%") % EventStreamConduit::count).str();
    conduit_idle_quantum_us = _conduit_idle_quantum_us;
}


bool EventStreamConduit::initialize(){
    
    
    // register a callback on
    event_stream->registerCallback(RESERVED_CODEC_CODE, boost::bind(&EventStreamConduit::handleCodecFromStream, shared_from_this(), _1));
        
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


void EventStreamConduit::handleControlEventFromConduit(shared_ptr<Event> evt){
    
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
    
    //std::cerr << "Responding to event forwarding request (id: " << event_id_datum.getString() << ", state: " << (bool)state_datum << ")" << std::endl;
    
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
            //std::cerr << "Now forwarding " << event_name << std::endl;
            events_to_forward.push_back(event_name);
            startForwardingEvent(event_name);
        }
    } else {
        
        if(event_iterator != events_to_forward.end()){
            events_to_forward.erase(event_iterator);
            rebuildStreamToConduitForwarding();
        }
    }
}


void EventStreamConduit::rebuildStreamToConduitForwarding(){
    list<string>::iterator i;
    
    //std::cerr << "rebuilding stream to conduit forwarding" << std::endl;
    
    // unregister any callbacks previously registered by this object
    event_stream->unregisterCallbacks(callback_key);
    
    // for each event named in events_to_forward (strings) register a callback
    // using the event stream interface
    for(i = events_to_forward.begin(); i != events_to_forward.end(); i++){
        startForwardingEvent(*i);
    }

}


void EventStreamConduit::startForwardingEvent(const std::string &tag_to_forward) {
    if(!tag_to_forward.empty()){
        // if the tag is listed
        if(stream_side_reverse_codec.find(tag_to_forward) != stream_side_reverse_codec.end()){
            int code = stream_side_reverse_codec[tag_to_forward];
            //std::cerr << "Registering forwarding callback for " << code << " on event stream" << std::endl;
            event_stream->registerCallback(code, boost::bind(&EventStreamConduit::sendData, shared_from_this(), _1), callback_key);
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
