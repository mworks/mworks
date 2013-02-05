/*!
 * @header IncomingEventListener.h
 *
 * @discussion Sets up a thread to listen to incoming events on a buffer.  Users register
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


BEGIN_NAMESPACE_MW


class IncomingEventListener : public EventListener {
    public:
        IncomingEventListener(shared_ptr<EventBuffer> _event_buffer, shared_ptr<EventStreamInterface>);
        ~IncomingEventListener();

    protected:
        // disallow copying
		IncomingEventListener(shared_ptr<EventBuffer> _event_buffer);
        IncomingEventListener(const IncomingEventListener&);
        IncomingEventListener& operator=(const IncomingEventListener&);
};


END_NAMESPACE_MW


#endif