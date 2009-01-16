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
namespace mw {
class OutgoingEventListener : public EventListener {
	
public:
	OutgoingEventListener(shared_ptr<BufferManager> manager, shared_ptr<EventHandler>);
	~OutgoingEventListener();
protected:
        // disallow copying
	OutgoingEventListener();
    OutgoingEventListener(const OutgoingEventListener&);
	OutgoingEventListener& operator=(const OutgoingEventListener&);
};
}
#endif
