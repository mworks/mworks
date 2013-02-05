/*!
* @header outgoingEventListener.h

 * 
 * @copyright MIT
 * @updated 9/6/07
 * @version 1.0.0
 */

#ifndef _OUTGOING_EVENT_LISTENER_H__
#define _OUTGOING_EVENT_LISTENER_H__

#include "EventListener.h"


BEGIN_NAMESPACE_MW


class OutgoingEventListener : public EventListener {
	
public:
	OutgoingEventListener(shared_ptr<EventBuffer> _event_buffer, shared_ptr<EventStreamInterface>);
	~OutgoingEventListener();
protected:
        // disallow copying
	OutgoingEventListener();
    OutgoingEventListener(const OutgoingEventListener&);
	OutgoingEventListener& operator=(const OutgoingEventListener&);
};


END_NAMESPACE_MW


#endif
