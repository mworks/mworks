/*
 *  CodecAwareConduit.cpp
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 12/10/09.
 *  Copyright 2009 Harvard University. All rights reserved.
 *
 */

#include "CodecAwareConduit.h"

CodecAwareConduit::CodecAwareConduit(shared_ptr<EventTransport> _transport) : SimpleConduit(_transport){  }


void CodecAwareConduit::transmitCodecEvent(){
    std::cerr << "transmitting codec" << endl;
    mw::Datum codec_datum(local_codec);
    sendData(RESERVED_CODEC_CODE, codec_datum);
}

void CodecAwareConduit::receiveCodecEvent(shared_ptr<Event> evt){
    std::cerr << "Receiving codec in conduit: " << this << std::endl;
    boost::mutex::scoped_lock lock(local_codec_lock);
    remote_codec = extract_simple_codec_map(evt);
    remote_reverse_codec = reverse_simple_codec_map(remote_codec);
    
    rebuildEventCallbacks();
    std::cerr << "Received codec in conduit: " << this << std::endl;

}

void CodecAwareConduit::rebuildEventCallbacks(){
    std::cerr << "rebuildEventCallbacks called in: " << this << std::endl;
    
    unregisterCallbacks(name_defined_callback_key);
    
    boost::mutex::scoped_lock lock(remote_codec_lock);
    map<string, EventCallback>::iterator i;
    for(i = callbacks_by_name.begin(); i != callbacks_by_name.end(); ++i){
        string evt_name = (*i).first;
        EventCallback cb = (*i).second;
        
        int incoming_code = remote_reverse_codec[evt_name];
        
        std::cerr << "registering callback for code: " << incoming_code << " to name: " << evt_name << " in: " << this << std::endl;
        
        
        // find out if that event name has a different code on this side of the conduit
        // if so, we'll want to create a translation callback
        if(local_reverse_codec.find(evt_name) != local_reverse_codec.end()){
            int local_code = local_reverse_codec[evt_name];
            EventCallback cb2 = boost::bind(&CodecAwareConduit::codeTranslatedCallback, shared_from_this(), _1, cb, local_code);
            registerCallback(incoming_code, cb2, string(name_defined_callback_key));
        } else {
            registerCallback(incoming_code, cb, string(name_defined_callback_key));
        }
    }
    
}

void CodecAwareConduit::codeTranslatedCallback(shared_ptr<Event> evt, EventCallback cb, int new_code){
    std::cerr << "calling code translated callback; new_code: " << new_code << " (old code: " << evt->getEventCode() << ")" << endl;
    evt->setEventCode(new_code);
    return cb(evt);
}


bool CodecAwareConduit::initialize(){
    std::cerr << "initializing: " << this << std::endl;
    //EventCallback cb = boost::bind(&CodecAwareConduit::receiveCodecEvent, this, _1);
    EventCallback cb = boost::bind(&CodecAwareConduit::receiveCodecEvent, shared_from_this(), _1);
    registerCallback(RESERVED_CODEC_CODE, cb);
    return SimpleConduit::initialize();
}

void CodecAwareConduit::registerCallbackByName(string event_name, EventCallback cb){
    callbacks_by_name[event_name] = cb;
    
    rebuildEventCallbacks();
}

void CodecAwareConduit::registerLocalEventCode(int code, string event_name){
    std::cerr << "registering local name: " << event_name << " to code: " << code << endl;
    boost::mutex::scoped_lock lock(local_codec_lock);
    local_codec[code] = event_name;
    
    local_reverse_codec = reverse_simple_codec_map(local_codec);
    transmitCodecEvent();
}
