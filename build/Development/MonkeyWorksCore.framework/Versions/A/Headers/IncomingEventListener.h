/*!
 * @header IncomingEventListener.h
 *
 * @discussion Sets up a thread to listen to incoming events.  Users register
 * an event handler with this object to deal with the events as they come.
 *
 * History:<BR/>
 * paul on 1/25/06 - Created.<BR/>
 * 
 * @copyright MIT
 * @updated 1/25/06
 * @version 1.0.0
 */

#ifndef _INCOMING_EVENT_LISTENER_H__
#define _INCOMING_EVENT_LISTENER_H__

#include "EventListener.h"
namespace mw {
class IncomingEventListener : public EventListener {
    public:
        IncomingEventListener(shared_ptr<BufferManager> manager, shared_ptr<EventHandler>);
        ~IncomingEventListener();

    protected:
        // disallow copying
		IncomingEventListener(shared_ptr<BufferManager> manager);
        IncomingEventListener(const IncomingEventListener&);
        IncomingEventListener& operator=(const IncomingEventListener&);
};
}
#endif