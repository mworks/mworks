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

void KeyedEventCallbackPair::operator=(const KeyedEventCallbackPair& cb_pair){
    key = cb_pair.key;
    callback = cb_pair.callback;
}

EventCallback KeyedEventCallbackPair::getCallback(){  
    if(callback != NULL){
        return *callback;
    } else {
        return boost::bind(&KeyedEventCallbackPair::dummyCallback, shared_from_this(), _1);
    }
}

string KeyedEventCallbackPair::getKey(){ return key; }


void KeyedEventCallbackPair::dummyCallback(shared_ptr<Event> evt){
    merror(M_SYSTEM_MESSAGE_DOMAIN, "Attempt to call an invalid callback functor");
}


void EventCallbackHandler::handleCallbacks(shared_ptr<Event> evt){
    
    if(evt == NULL){
        return;
    }
    
    int code = evt->getEventCode();
    
    //std::cerr << "handleCallbacks called in: " << this << " with event code " << code << std::endl;

    pair<EventCallbackMap::iterator, EventCallbackMap::iterator> itp;
    EventCallbackMap::iterator callback_iterator;
    
    CallbacksLock lock(*this);
    
    // issue any "always" callbacks
    // TODO: this can be made more efficient (e.g. search doesn't always need to be done)
    itp = callbacks_by_code.equal_range(ALWAYS_CALLBACK);
    for(callback_iterator = itp.first; callback_iterator != itp.second; ++callback_iterator){
        KeyedEventCallbackPair callback_pair = (*callback_iterator).second;
        EventCallback callback = (*callback_iterator).second.getCallback();
        callback(evt);
    }
    
    // Issue callbacks registered for particular codes
    itp = callbacks_by_code.equal_range(code);
    
    for(callback_iterator = itp.first; callback_iterator != itp.second; ++callback_iterator){
        KeyedEventCallbackPair callback_pair = (*callback_iterator).second;
        EventCallback callback = (*callback_iterator).second.getCallback();
        callback(evt);
        //std::cerr << "callback called for evt: " << code << " in: " << this << std::endl;
        
    }
    
}

void EventCallbackHandler::registerCallback(EventCallback cb, string callback_key){
    registerCallback(ALWAYS_CALLBACK, cb, callback_key);
}

void EventCallbackHandler::registerCallback(int code, EventCallback cb, string callback_key){
    CallbacksLock lock(*this);
    //std::cerr << "Registering callback for code: " << code << ", key: " << callback_key << std::endl;
    callbacks_by_code.insert(pair< int, KeyedEventCallbackPair >(code, KeyedEventCallbackPair(callback_key, cb)));
    codes_by_key.insert( pair<string, int>(callback_key, code) );
}


void EventCallbackHandler::unregisterCallbacksNoLocking(const std::string &key){
    return unregisterCallbacks(key, false);
}

void EventCallbackHandler::unregisterCallbacks(const std::string &key, bool locked) {
    
    boost::scoped_ptr<CallbacksLock> lock;
    if(locked){
        lock.reset(new CallbacksLock(*this));
    }
    
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
