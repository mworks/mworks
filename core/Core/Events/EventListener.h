/*!
* @header EventListener.h
 * 
 * @copyright MIT
 * @updated 9/5/07
 * @version 1.0.0
 */

#ifndef __EVENT_LISTENER_H__
#define __EVENT_LISTENER_H__


#include "EventBuffer.h"
#include "Event.h"
#include "EventStreamInterface.h"
#include "Scheduler.h"

#include "boost/enable_shared_from_this.hpp"
namespace mw {
class EventListener : public boost::enable_shared_from_this<EventListener>  {
protected:
	boost::mutex listenerLock;
	
	shared_ptr<EventBuffer> event_buffer;
	
	// listener for  events
	shared_ptr<EventBufferReader> reader;
	// the thread node
	shared_ptr<ScheduleTask> thread;
	// handle to an EventStreamInterface object
	shared_ptr<EventStreamInterface> handler;
	// are we in the middle of servicing
	bool servicing;
public:
       
    EventListener(shared_ptr<EventBuffer> _buffer, shared_ptr<EventStreamInterface> _stream_interface);
	virtual ~EventListener();
	virtual void startListener();
	virtual bool service();
	virtual void killListener();
	virtual void reset() {
		reader->reset();
	}
	
protected:
        // disallow copying
		EventListener(shared_ptr<EventBuffer> _event_buffer);
		EventListener(){ }
        EventListener(const EventListener&);
        virtual EventListener& operator=(const EventListener&);
};
}
#endif
