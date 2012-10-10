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


BEGIN_NAMESPACE_MW


void CodecAwareConduit::transmitCodecEvent(){
    // This is called internally from methods that already hold conduit_mutex
    
    //std::cerr << "transmitting codec" << endl;
    //mw::Datum codec_datum(local_codec);
    mw::Datum codec_datum = construct_simple_codec_datum_from_map(local_codec);
    sendData(RESERVED_CODEC_CODE, codec_datum);
}


void CodecAwareConduit::receiveCodecEvent(shared_ptr<Event> evt){
    // This is called from handleCallbacks, which already holds conduit_mutex
    
    remote_codec = extract_simple_codec_map(evt);
    remote_reverse_codec = reverse_simple_codec_map(remote_codec);
    
    rebuildEventCallbacks();
    
    remote_codec_cond.notify_all();
}


void CodecAwareConduit::receiveControlEvent(shared_ptr<Event> evt){
    // This is called from handleCallbacks, which already holds conduit_mutex
    
    Datum payload_type = evt->getData().getElement(M_SYSTEM_PAYLOAD_TYPE);
    if((int)payload_type == M_REQUEST_CODEC){
        transmitCodecEvent();
    }
}


void CodecAwareConduit::addEventCallback(const std::string &evt_name, EventCallback cb) {
    // This is called internally from methods that already hold conduit_mutex
    
    int incoming_code = M_UNDEFINED_EVENT_CODE;
        
    if (remote_reverse_codec.find(evt_name) != remote_reverse_codec.end()) {
        incoming_code = remote_reverse_codec[evt_name];
    } else {
        //std::cerr << "Cannot register callback for unknown name: " << evt_name << " (maybe codec hasn't been received yet?)" << endl;
        // send a request, just in case no one else has
        // sendData(SystemEventFactory::requestCodecControl());
        return;
    }
    
    //std::cerr << "registering callback for code: " << incoming_code << " to name: " << evt_name << " in: " << this << std::endl;
    
    // find out if that event name has a different code on this side of the conduit
    // if so, we'll want to create a translation callback
    int local_code = M_UNDEFINED_EVENT_CODE;
        
    if (local_reverse_codec.find(evt_name) != local_reverse_codec.end()) {
        local_code = local_reverse_codec[evt_name];
    }
    
    if (M_UNDEFINED_EVENT_CODE == local_code) {
        registerCallback(incoming_code, cb, string(name_defined_callback_key));
    } else {
        EventCallback cb2 = boost::bind(&CodecAwareConduit::codeTranslatedCallback, _1, cb, local_code);
        registerCallback(incoming_code, cb2, string(name_defined_callback_key));
    }
    
    // send a request to the other side of the conduit to begin forwarding this event
    sendData(SystemEventFactory::setEventForwardingControl(evt_name, true));
}


void CodecAwareConduit::rebuildEventCallbacks(){
    // This is called internally from methods that already hold conduit_mutex
    
    //std::cerr << "rebuildEventCallbacks called in: " << this << std::endl;
    
    unregisterCallbacks(name_defined_callback_key);
    
    for (map<string, EventCallback>::iterator i = callbacks_by_name.begin(); i != callbacks_by_name.end(); ++i) {
        string evt_name = (*i).first;
        EventCallback cb = (*i).second;
        addEventCallback(evt_name, cb);
    }
    
}


void CodecAwareConduit::codeTranslatedCallback(shared_ptr<Event> evt, EventCallback cb, int new_code){
    // This is a static method that does not need to hold conduit_mutex
    
    //std::cerr << "calling code translated callback; new_code: " << new_code << " (old code: " << evt->getEventCode() << ")" << endl;
    evt->setEventCode(new_code);
    return cb(evt);
}


void CodecAwareConduit::handleCallbacks(shared_ptr<Event> evt) {
    scoped_lock lock(conduit_mutex);
    SimpleConduit::handleCallbacks(evt);
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
    scoped_lock lock(conduit_mutex);
    
    if(event_name.size() == 0){
        throw SimpleException("Attempt to register callback for empty event name");
    }
    
    // We can't register a callback by name until we have the remote codec
    waitForRemoteCodec(lock);
    
    callbacks_by_name[event_name] = cb;
    addEventCallback(event_name, cb);
}


void CodecAwareConduit::registerLocalEventCode(int code, string event_name){
    scoped_lock lock(conduit_mutex);
    
    //std::cerr << "registering local name: " << event_name << " to code: " << code << endl;
    local_codec[code] = event_name;
    
    local_reverse_codec = reverse_simple_codec_map(local_codec);
    transmitCodecEvent();
    
}


map<int, string> CodecAwareConduit::getRemoteCodec() {
    scoped_lock lock(conduit_mutex);
    waitForRemoteCodec(lock);
    return remote_codec;
}


map<string, int> CodecAwareConduit::getRemoteReverseCodec() {
    scoped_lock lock(conduit_mutex);
    waitForRemoteCodec(lock);
    return remote_reverse_codec;
}


void CodecAwareConduit::waitForRemoteCodec(scoped_lock &lock) {
    if (remote_codec.empty()) {
        sendData(SystemEventFactory::requestCodecControl());
        
        boost::system_time timeout = boost::get_system_time() + boost::posix_time::milliseconds(CODEC_RESEND_TIMEOUT_MS);
        do {
            if (!remote_codec_cond.timed_wait(lock, timeout)) {
                throw SimpleException("Timeout while waiting for remote codec");
            }
        } while (remote_codec.empty());
    }
}


END_NAMESPACE_MW


























