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


EventStreamInterface::EventStreamInterface(MessageDomain _dom, bool locking){
    message_domain = _dom;
    recursively_lock_callbacks = locking;
}

EventStreamInterface::~EventStreamInterface() { }



void EventStreamInterface::handleCallbacks(shared_ptr<Event> event){
    
    if(event == NULL){
        return;
    }
    
    int code = event->getEventCode();
    EventCallbackMap::iterator callback_iterator = callbacks_by_code.find(code);
    for(; callback_iterator != callbacks_by_code.end(); callback_iterator++){
        EventCallback callback = (*callback_iterator).second.callback;
        callback(event);
    }
}

void EventStreamInterface::registerCallback(EventCallback cb, string callback_key){
    registerCallback(-1, cb, callback_key);
}

void EventStreamInterface::registerCallback(int code, EventCallback cb, string callback_key){
    boost::recursive_mutex::scoped_lock lock(callbacks_lock);
    callbacks_by_code.insert(pair< int, KeyedEventCallbackPair >(code, KeyedEventCallbackPair(callback_key, cb)));
    codes_by_key.insert( pair<string, int>(callback_key, code) );
}



void EventStreamInterface::unregisterCallbacks(const std::string &key) {
	
    EventCallbackKeyCodeMap::iterator code_iterator = codes_by_key.find(key);
    for(; code_iterator != codes_by_key.end(); code_iterator++){
        int code_to_search = (*code_iterator).second;
        
        EventCallbackMap::iterator callback_iterator = callbacks_by_code.find(code_to_search);
        for(; callback_iterator != callbacks_by_code.end(); callback_iterator++){
            KeyedEventCallbackPair callback_pair = (*callback_iterator).second;
            if(callback_pair.key == key){
                callbacks_by_code.erase(callback_iterator);
            }
        }
        
        codes_by_key.erase(code_iterator);
    }
}
