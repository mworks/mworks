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

#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include "MWorksMacros.h"
#include "Utilities.h"

#include "Event.h"

using std::string;


BEGIN_NAMESPACE_MW


typedef boost::function<void(shared_ptr<Event>)>  EventCallback;

// simple convenience class
class KeyedEventCallbackPair {

protected:
    
    shared_ptr<EventCallback> callback;
    string key;
    
public:
    
    KeyedEventCallbackPair();
    
    KeyedEventCallbackPair(string _key, EventCallback _callback);    
    EventCallback getCallback();
    string getKey();    
    
private:
    
    static void dummyCallback(shared_ptr<Event> evt);
    
};

typedef std::multimap<int, KeyedEventCallbackPair> EventCallbackMap;

#define DEFAULT_CALLBACK_KEY    "<default>"
#define ALWAYS_CALLBACK_KEY     -1
#define ALWAYS_CALLBACK     ALWAYS_CALLBACK_KEY



class EventCallbackHandler {
    
private:
    
    // A dictionary of callback functors by code
    EventCallbackMap                callbacks_by_code;
    
    // Thread safety measures
    boost::mutex callbacks_lock;
    boost::recursive_mutex recursive_callbacks_lock;
    const bool recursively_lock_callbacks;
   
public:
    
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
    
    virtual void unregisterCallbacks(const string &callback_key = DEFAULT_CALLBACK_KEY);
    
    // callback dispatch
    virtual void handleCallbacks(shared_ptr<Event> evt);
    
};


END_NAMESPACE_MW


#endif
