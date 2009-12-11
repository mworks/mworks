/*
 *  EventCallbackHandler.h
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 12/9/09.
 *  Copyright 2009 Harvard University. All rights reserved.
 *
 */

#ifndef EVENT_CALLBACK_HANDLER_H_
#define EVENT_CALLBACK_HANDLER_H_

#include <boost/function.hpp>
#include <string>
#include <map>
#include <hash_map.h>
#include <multimap.h>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>

#include "Event.h"

using namespace boost;
using namespace std;
using namespace mw;

typedef boost::function<void(shared_ptr<Event>)>  EventCallback;

// simple convenience class
class KeyedEventCallbackPair{
public:
    EventCallback callback;
    string key;
    
    KeyedEventCallbackPair(){ }
    
    KeyedEventCallbackPair(string _key, EventCallback _callback){
        key = _key;
        callback = _callback;
    }
    
    void operator=(const KeyedEventCallbackPair& cb_pair){
        key = cb_pair.key;
        callback = cb_pair.callback;
    }
    
};


typedef hash_multimap<int, KeyedEventCallbackPair>  EventCallbackMap;
typedef multimap<string, int>      EventCallbackKeyCodeMap;

#define DEFAULT_CALLBACK_KEY    "<default>"
#define ALWAYS_CALLBACK_KEY     -1
#define ALWAYS_CALLBACK     ALWAYS_CALLBACK_KEY



class EventCallbackHandler {
    
protected:
    
    // A dictionary of callback functors by code
    EventCallbackMap                callbacks_by_code;
    
    // A dictionary of event codes indexed by callback keys
    EventCallbackKeyCodeMap         codes_by_key;
    
    // Thread safety measures
    boost::recursive_mutex callbacks_lock;
    bool recursively_lock_callbacks;
    
    
public:
    
    EventCallbackHandler(bool locking){
        recursively_lock_callbacks = locking;
    }
    
    virtual ~EventCallbackHandler(){ 
        // grab this lock to ensure that anyone else who needs it
        // is done
        boost::recursive_mutex::scoped_lock lock(callbacks_lock);
    }
    
    virtual void registerCallback(EventCallback cb, string callback_key = DEFAULT_CALLBACK_KEY);
    virtual void registerCallback(int code, EventCallback cb, string callback_key = DEFAULT_CALLBACK_KEY);
    //virtual void registerCallback(string tagname, EventCallback cb, string callback_key = DEFAULT_CALLBACK_KEY);
    
    virtual void unregisterCallbacks(const string &callback_key = DEFAULT_CALLBACK_KEY);
    
    // callback dispatch
    virtual void handleCallbacks(shared_ptr<Event> evt);
    
};

#endif
