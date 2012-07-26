/*
 *  EventCallbackHandler.h
 *  MWorksCore
 *
 *  Created by David Cox on 12/9/09.
 *  Copyright 2009 Harvard University. All rights reserved.
 *
 */

#ifndef EVENT_CALLBACK_HANDLER_H_
#define EVENT_CALLBACK_HANDLER_H_

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <string>
#include <map>

#ifdef USE_HASH_MAP_IN_CALLBACK_HANDLER
#include <ext/hash_map>
#endif

#include <map>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include "MWorksMacros.h"
#include "Utilities.h"

#include "Event.h"

using namespace boost;
using namespace std;
using namespace __gnu_cxx;


BEGIN_NAMESPACE_MW


typedef boost::function<void(shared_ptr<Event>)>  EventCallback;

// simple convenience class
class KeyedEventCallbackPair : public enable_shared_from_this<KeyedEventCallbackPair>{

protected:
    
    shared_ptr<EventCallback> callback;
    string key;
    
public:
    
    KeyedEventCallbackPair();
    
    KeyedEventCallbackPair(string _key, EventCallback _callback);    
    void operator=(const KeyedEventCallbackPair& cb_pair);    
    EventCallback getCallback();    
    string getKey();    
    
    void dummyCallback(shared_ptr<Event> evt);    
};

#ifdef  USE_HASH_MAP_IN_CALLBACK_HANDLER
typedef hash_multimap<int, KeyedEventCallbackPair>  EventCallbackMap;
#else
typedef multimap<int, KeyedEventCallbackPair> EventCallbackMap;
#endif

typedef multimap<string, int>      EventCallbackKeyCodeMap;

#define DEFAULT_CALLBACK_KEY    "<default>"
#define ALWAYS_CALLBACK_KEY     -1
#define ALWAYS_CALLBACK     ALWAYS_CALLBACK_KEY



class EventCallbackHandler {
    
private:
    
    // A dictionary of callback functors by code
    EventCallbackMap                callbacks_by_code;
    
    // A dictionary of event codes indexed by callback keys
    EventCallbackKeyCodeMap         codes_by_key;
    
    // Thread safety measures
    boost::mutex callbacks_lock;
    boost::recursive_mutex recursive_callbacks_lock;
    const bool recursively_lock_callbacks;
   
protected:
    
    class CallbacksLock {
    public:
        CallbacksLock(EventCallbackHandler &h) : handler(h) {
            if (handler.recursively_lock_callbacks) {
                handler.recursive_callbacks_lock.lock();
            } else {
                handler.callbacks_lock.lock();
            }
        }
        
        ~CallbacksLock() {
            if (handler.recursively_lock_callbacks){
                handler.recursive_callbacks_lock.unlock();
            } else {
                handler.callbacks_lock.unlock();
            }
        }
        
    private:
        EventCallbackHandler &handler;
    };
    
public:
    
    EventCallbackHandler(bool locking) : recursively_lock_callbacks(locking) { }
    
    virtual ~EventCallbackHandler(){ 
        // grab these locks to ensure that anyone else who needs them
        // is done
        boost::recursive_mutex::scoped_lock recursive_lock(recursive_callbacks_lock);
        boost::mutex::scoped_lock lock(callbacks_lock);
        
    }
        
    virtual void registerCallback(EventCallback cb, string callback_key = DEFAULT_CALLBACK_KEY);
    virtual void registerCallback(int code, EventCallback cb, string callback_key = DEFAULT_CALLBACK_KEY);
    //virtual void registerCallback(string tagname, EventCallback cb, string callback_key = DEFAULT_CALLBACK_KEY);
    
    
    virtual void unregisterCallbacksNoLocking(const std::string &key);
    virtual void unregisterCallbacks(const string &callback_key = DEFAULT_CALLBACK_KEY, bool locked = true);
    
    // callback dispatch
    virtual void handleCallbacks(shared_ptr<Event> evt);
    
};


END_NAMESPACE_MW


#endif
