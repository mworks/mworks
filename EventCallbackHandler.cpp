/*
 *  EventCallbackHandler.cpp
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 12/9/09.
 *  Copyright 2009 Harvard University. All rights reserved.
 *
 */

#include "EventCallbackHandler.h"

void EventCallbackHandler::handleCallbacks(shared_ptr<Event> evt){
    
    if(evt == NULL){
        return;
    }
    
    std::cerr << "handleCallbacks called in: " << this << std::endl;

    
    boost::recursive_mutex::scoped_lock lock(callbacks_lock);
    int code = evt->getEventCode();
    
    // issue any "always" callbacks
    // TODO: this can be made more efficient (e.g. search doesn't always need to be done)
    pair<EventCallbackMap::iterator, EventCallbackMap::iterator> itp = callbacks_by_code.equal_range(ALWAYS_CALLBACK);
    EventCallbackMap::iterator callback_iterator;
    for(callback_iterator = itp.first; callback_iterator != itp.second; ++callback_iterator){
        EventCallback callback = (*callback_iterator).second.callback;
        callback(evt);
    }
    
    itp = callbacks_by_code.equal_range(code);
    
    for(callback_iterator = itp.first; callback_iterator != itp.second; ++callback_iterator){
        EventCallback callback = (*callback_iterator).second.callback;
        callback(evt);
        std::cerr << "callback called for evt: " << code << " in: " << this << std::endl;
        
    }
}

void EventCallbackHandler::registerCallback(EventCallback cb, string callback_key){
    registerCallback(ALWAYS_CALLBACK, cb, callback_key);
}

void EventCallbackHandler::registerCallback(int code, EventCallback cb, string callback_key){
    boost::recursive_mutex::scoped_lock lock(callbacks_lock);
    callbacks_by_code.insert(pair< int, KeyedEventCallbackPair >(code, KeyedEventCallbackPair(callback_key, cb)));
    codes_by_key.insert( pair<string, int>(callback_key, code) );
}



void EventCallbackHandler::unregisterCallbacks(const std::string &key) {
    boost::recursive_mutex::scoped_lock lock(callbacks_lock);
    
    // For now, just do a straight linear time search
    EventCallbackMap::iterator callback_iterator;
    for(callback_iterator = callbacks_by_code.begin(); callback_iterator != callbacks_by_code.end(); ++callback_iterator){
        if((*callback_iterator).second.key == key){
            callbacks_by_code.erase(callback_iterator);
        }
    }
}
