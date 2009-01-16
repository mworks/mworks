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
#include "EventHandler.h"
#include "Scheduler.h"

#include "boost/enable_shared_from_this.hpp"
namespace mw {
class EventListener : public enable_shared_from_this<EventListener>  {
protected:
	boost::mutex listenerLock;
	
	shared_ptr<BufferManager> buffer_manager;
	
	// listener for  events
	shared_ptr<EventBufferReader> reader;
	// the thread node
	shared_ptr<ScheduleTask> thread;
	// handle to an eventhandler object
	shared_ptr<EventHandler> handler;
	// are we in the middle of servicing
	bool servicing;
public:
        
	virtual ~EventListener();
	virtual void startListener();
	virtual bool service();
	virtual void killListener();
	virtual void reset() {
		reader->reset();
	}
	
protected:
        // disallow copying
		EventListener(shared_ptr<BufferManager> manager);
		EventListener(){ }
        EventListener(const EventListener&);
	virtual EventListener& operator=(const EventListener&);
};
}
#endif
