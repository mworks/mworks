/*!
 * @header EventStreamInterface.h
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
#include <boost/enable_shared_from_this.hpp>
#include <multimap.h>
#include <hash_map.h>

namespace mw {

using namespace std;


    
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

    
    
class EventStreamInterface : public enable_shared_from_this<EventStreamInterface> {
    
protected:
    
    // domain where this object will report errors
    MessageDomain message_domain; 
  
    // A dictionary of callback functors by code
    EventCallbackMap                callbacks_by_code;
    
    // A dictionary of event codes indexed by callback keys
    EventCallbackKeyCodeMap         codes_by_key;
    
    // Thread safety measures
    boost::recursive_mutex callbacks_lock;
    bool recursively_lock_callbacks;
    
public:
    EventStreamInterface(MessageDomain dom, bool locking);
    virtual ~EventStreamInterface();
    
    
    // basic interface: must have methods to putEvent into the stream
    // and to handle events received from the stream
    virtual void handleEvent(shared_ptr<Event> e) = 0;
    virtual void putEvent(shared_ptr<Event> e) = 0;

        
    // callback registration and dispatch
    virtual void handleCallbacks(shared_ptr<Event> evt);


    virtual void registerCallback(EventCallback cb, string callback_key = DEFAULT_CALLBACK_KEY);
    virtual void registerCallback(int code, EventCallback cb, string callback_key = DEFAULT_CALLBACK_KEY);
    //virtual void registerCallback(string tagname, EventCallback cb, string callback_key = DEFAULT_CALLBACK_KEY);

    virtual void unregisterCallbacks(const string &callback_key);

        
};
    

// An event stream interface that maintains its own outgoing codec
class SelfContainedEventStreamInterface : public EventStreamInterface {
    
};
    
}
#endif

