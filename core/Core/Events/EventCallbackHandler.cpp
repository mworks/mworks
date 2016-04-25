/*
 *  EventCallbackHandler.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 12/9/09.
 *  Copyright 2009 Harvard University. All rights reserved.
 *
 */

#include "EventCallbackHandler.h"

#include <boost/scoped_ptr.hpp>


BEGIN_NAMESPACE_MW


KeyedEventCallbackPair::KeyedEventCallbackPair(){ 
    key = "<invalid>";
    callback = shared_ptr<EventCallback>();
}


KeyedEventCallbackPair::KeyedEventCallbackPair(string _key, EventCallback _callback){
    key = _key;
    callback = shared_ptr<EventCallback>(new EventCallback(_callback));
}

EventCallback KeyedEventCallbackPair::getCallback(){  
    if(callback != NULL){
        return *callback;
    } else {
        return &dummyCallback;
    }
}

string KeyedEventCallbackPair::getKey(){ return key; }


void KeyedEventCallbackPair::dummyCallback(shared_ptr<Event> evt){
    merror(M_SYSTEM_MESSAGE_DOMAIN, "Attempt to call an invalid callback functor");
}


void EventCallbackHandler::handleCallbacks(shared_ptr<Event> evt) {
    if (!evt) {
        return;
    }
    
    //
    // Some callbacks will want to unregister existing callbacks and/or register new callbacks.
    // To support this, we extract all matching callbacks for the given event to a local vector
    // before invoking any of them, so that callbacks_by_code won't be modified while we're
    // iterating through it.
    //
    std::vector<EventCallback> matchingCallbacks;
    {
        scoped_lock lock(callbacks_lock);
        
        {
            // Extract "always" callbacks
            // TODO: this can be made more efficient (e.g. search doesn't always need to be done)
            auto alwaysMatches = callbacks_by_code.equal_range(ALWAYS_CALLBACK);
            for (auto iter = alwaysMatches.first; iter != alwaysMatches.second; iter++) {
                matchingCallbacks.push_back(iter->second.getCallback());
            }
        }
        
        {
            // Extract callbacks registered for the given event's code
            auto codeMatches = callbacks_by_code.equal_range(evt->getEventCode());
            for (auto iter = codeMatches.first; iter != codeMatches.second; iter++) {
                matchingCallbacks.push_back(iter->second.getCallback());
            }
        }
    }
    
    // Issue callbacks
    for (auto &callback : matchingCallbacks) {
        callback(evt);
    }
}

void EventCallbackHandler::registerCallback(EventCallback cb, string callback_key){
    registerCallback(ALWAYS_CALLBACK, cb, callback_key);
}

void EventCallbackHandler::registerCallback(int code, EventCallback cb, string callback_key){
    scoped_lock lock(callbacks_lock);
    //std::cerr << "Registering callback for code: " << code << ", key: " << callback_key << std::endl;
    callbacks_by_code.insert(pair< int, KeyedEventCallbackPair >(code, KeyedEventCallbackPair(callback_key, cb)));
}


void EventCallbackHandler::unregisterCallbacks(const std::string &key) {
    scoped_lock lock(callbacks_lock);
    
    //std::cerr << "Unregistering callbacks for key: " << key << std::endl;
    // For now, just do a straight linear time search
    EventCallbackMap::iterator callback_iterator;
    vector<EventCallbackMap::iterator> nodes_to_erase;
    
    for(callback_iterator = callbacks_by_code.begin(); callback_iterator != callbacks_by_code.end(); ++callback_iterator){
        if((*callback_iterator).second.getKey() == key){
            //std::cerr << "\tunregistering code: " << (*callback_iterator).first << std::endl;
            nodes_to_erase.push_back(callback_iterator);
        }
    }
    
    vector<EventCallbackMap::iterator>::iterator erase_iterator;
    for(erase_iterator = nodes_to_erase.begin(); erase_iterator != nodes_to_erase.end(); ++erase_iterator){
        callbacks_by_code.erase(*erase_iterator);
    }
}


END_NAMESPACE_MW
