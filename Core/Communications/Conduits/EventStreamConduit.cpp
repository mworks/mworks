/*
 *  EventStreamInterfaceConduit.cpp
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 10/30/09.
 *  Copyright 2009 Harvard University. All rights reserved.
 *
 *  @discussion  The EventStreamInterfaceConduit bridges from an interprocess transport
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
#include "ControlEventFactory.h"
#include "StandardVariables.h"

using namespace mw;

EventStreamInterfaceConduit::EventStreamInterfaceConduit(shared_ptr<EventTransport> _transport, shared_ptr<EventStreamInterface> _handler) : Conduit(_transport){
    event_handler = _handler;
}


bool EventStreamInterfaceConduit::initialize(){
    
    
    // register a callback on the *event_handler* for the codec
    event_handler->registerCallback(RESERVED_CODEC_CODE, boost::bind(&EventStreamInterfaceConduit::handleCodec, shared_from_this(), _1));
    
    // register a callback on *this conduit* for control events from the other side
//    this->registerCallback(GlobalSystemEventVariable->getCodecCode(), boost::bind(&EventStreamInterfaceConduit::handleControlEvent, shared_from_this(), _1)); 
    
    
    // request a codec from the event_handler (a proxy to the rest of MW)
    event_handler->putEvent(ControlEventFactory::requestCodecControl());
    
    return Conduit::initialize();
  
}


void EventStreamInterfaceConduit::rebuildCodesToForward(){
    list<string>::iterator i;
    
    codes_to_forward.clear();
    
    // for each event named in events_to_forward (strings)
    for(i = events_to_forward.begin(); i != events_to_forward.end(); i++){
        // that tag that we need to forward
        string tag_to_forward = *i;
        if(!tag_to_forward.empty()){
            // if the tag is listed
            if(reverse_codec_map.find(tag_to_forward) != reverse_codec_map.end()){
                int code = reverse_codec_map[tag_to_forward];
                codes_to_forward.push_back(code);
            }
        }
    }
}
