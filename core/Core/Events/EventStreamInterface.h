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
#include <map>

#include "EventCallbackHandler.h"

namespace mw {


class EventStreamInterface : public EventCallbackHandler, public boost::enable_shared_from_this<EventStreamInterface> {
    
protected:
    
    // domain where this object will report errors
    MessageDomain message_domain; 
  
public:
    EventStreamInterface(MessageDomain dom, bool locking);
    virtual ~EventStreamInterface();
    
    
    // basic interface: must have methods to putEvent into the stream
    // and to handle events received from the stream
    virtual void handleEvent(shared_ptr<Event> e) = 0;
    virtual void putEvent(shared_ptr<Event> e) = 0;
};
    

    
}
#endif

