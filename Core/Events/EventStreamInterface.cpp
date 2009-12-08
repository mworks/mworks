/**
 * EventStreamInterface.cpp
 *
 * History:
 * paul on 1/27/06 - Created.
 *
 * Copyright 2006 MIT. All rights reserved.
 */

#include "EventStreamInterface.h"
using namespace mw;

using namespace std;

#define ALWAYS_CALLBACK -1


EventStreamInterface::EventStreamInterface(MessageDomain _dom, bool locking){
    message_domain = _dom;
    recursively_lock_callbacks = locking;
}

EventStreamInterface::~EventStreamInterface() { }



void EventStreamInterface::handleCallbacks(shared_ptr<Event> event){
    
    if(event == NULL){
        return;
    }
    boost::recursive_mutex::scoped_lock lock(callbacks_lock);
    int code = event->getEventCode();
    
    // TODO: I think that this is where the funky is
    //    EventCallbackMap::iterator callback_iterator = callbacks_by_code.find(code);
    //    for(; callback_iterator != callbacks_by_code.end(); callback_iterator++){

    // issue any "always" callbacks
    // TODO: this can be made more efficient (e.g. search doesn't always need to be done)
    pair<EventCallbackMap::iterator, EventCallbackMap::iterator> itp = callbacks_by_code.equal_range(ALWAYS_CALLBACK);
    EventCallbackMap::iterator callback_iterator;
    for(callback_iterator = itp.first; callback_iterator != itp.second; ++callback_iterator){
        EventCallback callback = (*callback_iterator).second.callback;
        callback(event);
    }
    
    itp = callbacks_by_code.equal_range(code);

    for(callback_iterator = itp.first; callback_iterator != itp.second; ++callback_iterator){
        EventCallback callback = (*callback_iterator).second.callback;
        callback(event);
    }
}

void EventStreamInterface::registerCallback(EventCallback cb, string callback_key){
    registerCallback(ALWAYS_CALLBACK, cb, callback_key);
}

void EventStreamInterface::registerCallback(int code, EventCallback cb, string callback_key){
    boost::recursive_mutex::scoped_lock lock(callbacks_lock);
    callbacks_by_code.insert(pair< int, KeyedEventCallbackPair >(code, KeyedEventCallbackPair(callback_key, cb)));
    codes_by_key.insert( pair<string, int>(callback_key, code) );
}



void EventStreamInterface::unregisterCallbacks(const std::string &key) {
    boost::recursive_mutex::scoped_lock lock(callbacks_lock);
    //EventCallbackKeyCodeMap::iterator code_iterator = codes_by_key.find(key);
    //for(; code_iterator != codes_by_key.end(); code_iterator++){

    
    // For now, just do a straight linear time search
    EventCallbackMap::iterator callback_iterator;
    for(callback_iterator = callbacks_by_code.begin(); callback_iterator != callbacks_by_code.end(); ++callback_iterator){
        if((*callback_iterator).second.key == key){
            callbacks_by_code.erase(callback_iterator);
        }
    }
    
   /* 
    pair<EventCallbackKeyCodeMap::iterator, EventCallbackKeyCodeMap::iterator> itp = codes_by_key.equal_range(key);
    EventCallbackKeyCodeMap::iterator code_iterator;
    for(code_iterator = itp.first; code_iterator != itp.second; ++code_iterator){
        int code_to_search = (*code_iterator).second;

//        EventCallbackMap::iterator callback_iterator = callbacks_by_code.find(code_to_search);
//        for(; callback_iterator != callbacks_by_code.end(); callback_iterator++){
            
        pair<EventCallbackMap::iterator, EventCallbackMap::iterator> itp2 = callbacks_by_code.equal_range(code_to_search);
        EventCallbackMap::iterator callback_iterator;
        for(callback_iterator = itp2.first; callback_iterator != itp2.second; ++callback_iterator){
            
            if((*callback_iterator).second.key == key){
                callbacks_by_code.erase(callback_iterator);
            }
        }
        
        codes_by_key.erase(code_iterator);
    }*/
}
