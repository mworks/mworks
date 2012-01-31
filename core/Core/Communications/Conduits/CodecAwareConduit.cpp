/*
 *  CodecAwareConduit.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 12/10/09.
 *  Copyright 2009 Harvard University. All rights reserved.
 *
 */

#include "CodecAwareConduit.h"
#include "SystemEventFactory.h"
#include "StandardVariables.h"

CodecAwareConduit::CodecAwareConduit(shared_ptr<EventTransport> _transport,
                                     bool correct_incoming_timestamps) : 
    SimpleConduit(_transport, correct_incoming_timestamps),
    local_codec_code_counter(0)
{  

}


void CodecAwareConduit::transmitCodecEvent(){
    //std::cerr << "transmitting codec" << endl;
    //mw::Datum codec_datum(local_codec);
    mw::Datum codec_datum = construct_simple_codec_datum_from_map(local_codec);
    sendData(RESERVED_CODEC_CODE, codec_datum);
}

void CodecAwareConduit::receiveCodecEvent(shared_ptr<Event> evt){
    //std::cerr << "Receiving codec in conduit: " << this << std::endl;
    boost::mutex::scoped_lock lock(local_codec_lock);
    remote_codec = extract_simple_codec_map(evt);
    remote_reverse_codec = reverse_simple_codec_map(remote_codec);
    
    rebuildEventCallbacks();
    //std::cerr << "Received codec in conduit: " << this << std::endl;

}

void CodecAwareConduit::receiveControlEvent(shared_ptr<Event> evt){
    
    Datum payload_type = evt->getData().getElement(M_SYSTEM_PAYLOAD_TYPE);
    if((int)payload_type == M_REQUEST_CODEC){
        transmitCodecEvent();
    }
}

void CodecAwareConduit::rebuildEventCallbacks(){
    //std::cerr << "rebuildEventCallbacks called in: " << this << std::endl;
    
    unregisterCallbacks(name_defined_callback_key);
    
    boost::mutex::scoped_lock lock(remote_codec_lock);
    map<string, EventCallback>::iterator i;
    for(i = callbacks_by_name.begin(); i != callbacks_by_name.end(); ++i){
        string evt_name = (*i).first;
        EventCallback cb = (*i).second;
        
        int incoming_code;
        if(remote_reverse_codec.find(evt_name) != remote_reverse_codec.end()){
            incoming_code = remote_reverse_codec[evt_name];
        } else {
            //std::cerr << "Cannot register callback for unknown name: " << evt_name << " (maybe codec hasn't been received yet?)" << endl;
            // send a request, just in case no one else has
            // sendData(SystemEventFactory::requestCodecControl());
            continue;
        }
        
        //std::cerr << "registering callback for code: " << incoming_code << " to name: " << evt_name << " in: " << this << std::endl;
        
        
        // find out if that event name has a different code on this side of the conduit
        // if so, we'll want to create a translation callback
        if(local_reverse_codec.find(evt_name) != local_reverse_codec.end()){
            int local_code = local_reverse_codec[evt_name];
            EventCallback cb2 = boost::bind(&CodecAwareConduit::codeTranslatedCallback, shared_from_this(), _1, cb, local_code);
            registerCallback(incoming_code, cb2, string(name_defined_callback_key));
        } else {
            registerCallback(incoming_code, cb, string(name_defined_callback_key));
        }
        
        // send a request to the other side of the conduit to begin forwarding this event
        sendData(SystemEventFactory::setEventForwardingControl(evt_name, true));
    }
    
}

void CodecAwareConduit::codeTranslatedCallback(shared_ptr<Event> evt, EventCallback cb, int new_code){
    //std::cerr << "calling code translated callback; new_code: " << new_code << " (old code: " << evt->getEventCode() << ")" << endl;
    evt->setEventCode(new_code);
    return cb(evt);
}


bool CodecAwareConduit::initialize(){
    //std::cerr << "initializing: " << this << std::endl;
    //EventCallback cb = boost::bind(&CodecAwareConduit::receiveCodecEvent, this, _1);
    EventCallback cb = boost::bind(&CodecAwareConduit::receiveCodecEvent, shared_from_this(), _1);
    registerCallback(RESERVED_CODEC_CODE, cb);
    registerCallback(RESERVED_SYSTEM_EVENT_CODE, boost::bind(&CodecAwareConduit::receiveControlEvent, shared_from_this(), _1));
    bool okayp = SimpleConduit::initialize();
    
    if(okayp){
        sendData(SystemEventFactory::requestCodecControl());
    }
    
    return okayp;
}

void CodecAwareConduit::registerCallbackByName(string event_name, EventCallback cb){
    if(event_name.size() == 0){
        throw SimpleException("Attempt to register callback for empty event name");
    }
    callbacks_by_name[event_name] = cb;
    
    rebuildEventCallbacks();
    
    //std::cerr << "Sending event forwarding event for " << event_name << std::endl;
    sendData(SystemEventFactory::setEventForwardingControl(event_name, true));
}

void CodecAwareConduit::registerLocalEventCode(int code, string event_name){
    //std::cerr << "registering local name: " << event_name << " to code: " << code << endl;
    boost::mutex::scoped_lock lock(local_codec_lock);
    local_codec[code] = event_name;
    
    local_reverse_codec = reverse_simple_codec_map(local_codec);
    transmitCodecEvent();
    
}


map<int, string> CodecAwareConduit::getRemoteCodec(){
    bool empty = true;
    { 
        boost::mutex::scoped_lock lock(local_codec_lock);
        empty = remote_codec.empty();
    }
    
    if(empty){
        sendData(SystemEventFactory::requestCodecControl());
        shared_ptr<Clock> c = Clock::instance(false);
        if(c != NULL){
            c->sleepMS(CODEC_RESEND_TIMEOUT_MS);
        }
    }
    
    return remote_codec; 
}

map<string, int> CodecAwareConduit::getRemoteReverseCodec(){  return remote_reverse_codec; }
