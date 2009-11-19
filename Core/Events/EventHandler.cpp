/**
 * EventHandler.cpp
 *
 * History:
 * paul on 1/27/06 - Created.
 *
 * Copyright 2006 MIT. All rights reserved.
 */

#include "EventHandler.h"
using namespace mw;

using namespace std;


EventHandler::EventHandler(MessageDomain _dom, bool locking){
    message_domain = _dom;
    recursively_lock_callbacks = locking;
}

EventHandler::~EventHandler() { }



shared_ptr<Variable> EventHandler::getVariable(const std::string &tag) {
	if(registry != NULL){
        return registry->getVariable(tag);
    } else {
        return shared_ptr<Variable>();
    }
}

shared_ptr<Variable> EventHandler::getVariable(const int code) {
	if(registry != NULL){
        return registry->getVariable(code);
    } else {
        return shared_ptr<Variable>();
    }
}

int EventHandler::lookupCodeForTag(const std::string &tag) {
	shared_ptr<Variable> var = getVariable(tag);
	if(var) {
		return var->getCodecCode();
	} else {
		return -1;
	}
}

std::vector<std::string> EventHandler::getVariableNames() {
	if(registry != NULL){
        return registry->getVariableTagnames();
    } else {
        return std::vector<std::string>();
    }
}

void EventHandler::handleCallbacks(shared_ptr<Event> event){
    
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

void EventHandler::registerCallback(EventCallback cb, string callback_key){
    registerCallback(-1, cb, callback_key);
}

void EventHandler::registerCallback(int code, EventCallback cb, string callback_key){
    boost::recursive_mutex::scoped_lock lock(callbacks_lock);
    callbacks_by_code.insert(pair< int, KeyedEventCallbackPair >(code, KeyedEventCallbackPair(callback_key, cb)));
    codes_by_key.insert( pair<string, int>(callback_key, code) );
}

void EventHandler::registerCallback(string tagname, EventCallback cb, string callback_key){
    int code = lookupCodeForTag(tagname);   
    registerCallback(code, cb, callback_key);
}



void EventHandler::unregisterCallbacks(const std::string &key) {
	
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
