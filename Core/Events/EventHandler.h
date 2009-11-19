/*!
 * @header EventHandler.h
 *
 * @discussion An object for interfacing with a stream of events.
 * This object provides an abstract interface for sending and receiving
 * events from a stream (this could be a network connection, or an interprocess
 * conduit, or simply an event buffer), along with infrastructure for
 * registering callbacks by event code or name, including transparent handling
 * of codec transmission, interpretation, etc.
 * While this object provides methods for dispatching callbacks in response to
 * events, it does not provide any sort of polling loop, or similar functionality.
 * This is the job of a subclass or separate object.
 *
 * History:<BR/>
 * paul on 1/27/06 - Created.<BR/>
 * 
 * @copyright MIT
 * @updated 1/27/06
 * @version 1.0.0
 */

#ifndef _EVENT_HANDLER_H__
#define _EVENT_HANDLER_H__

#include "Event.h"
#include "GenericVariable.h"
#include "VariableRegistry.h"
#include "CoreEventFunctor.h"
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <multimap.h>
#include <hash_map.h>

namespace mw {

using namespace std;
//typedef std::vector <shared_ptr<GenericEventFunctor> > CallbacksForVar;

    
typedef boost::function<void(shared_ptr<Event>)>  EventCallback;

// simple convenience class
class KeyedEventCallbackPair{
public:
    EventCallback callback;
    string key;
    
    KeyedEventCallbackPair(string _key, EventCallback _callback){
        key = key;
        callback = _callback;
    }
    
};
    
typedef hash_multimap<int, KeyedEventCallbackPair>  EventCallbackMap;
typedef multimap<string, int>      EventCallbackKeyCodeMap;

#define DEFAULT_CALLBACK_KEY    "<default>"
#define ALWAYS_CALLBACK_KEY     -1

    
    
    
class EventHandler {
    
protected:
    
    
    MessageDomain message_domain; // domain where this object will report errors
  
    EventCallbackMap   callbacks_by_code;
    EventCallbackKeyCodeMap         codes_by_key;
    
    boost::recursive_mutex callbacks_lock;
    bool recursively_lock_callbacks;

    shared_ptr<VariableRegistry> registry;

public:
    EventHandler(MessageDomain dom, bool locking);
    virtual ~EventHandler();
    virtual void handleEvent(shared_ptr<Event> e) = 0;
    virtual void putEvent(shared_ptr<Event> e) = 0;

    virtual shared_ptr<Variable> getVariable(const string &tag);
    virtual shared_ptr<Variable> getVariable(const int code);
    virtual int lookupCodeForTag(const string &tag);
    virtual std::vector<string> getVariableNames();
    
    
    virtual void handleCallbacks(shared_ptr<Event> evt);

    virtual void registerCallback(EventCallback cb, string callback_key = DEFAULT_CALLBACK_KEY);
    virtual void registerCallback(int code, EventCallback cb, string callback_key = DEFAULT_CALLBACK_KEY);
    virtual void registerCallback(string tagname, EventCallback cb, string callback_key = DEFAULT_CALLBACK_KEY);

//    virtual void registerCallback(shared_ptr<GenericEventFunctor> gef);
//    virtual void registerCallback(shared_ptr<GenericEventFunctor> gef, int code);
//    virtual void registerCallback(shared_ptr<GenericEventFunctor> gef,string tagname);
    
    virtual void unregisterCallbacks(const string &callback_key);

        
};
}
#endif

