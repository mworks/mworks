/*!
* @header EventListener.h
 * 
 * @copyright MIT
 * @updated 9/5/07
 * @version 1.0.0
 */

#ifndef __EVENT_LISTENER_H__
#define __EVENT_LISTENER_H__

#include <thread>

#include "EventBuffer.h"
#include "EventStreamInterface.h"


BEGIN_NAMESPACE_MW


class EventListener {
    
public:
    EventListener(const boost::shared_ptr<EventBuffer> &buffer,
                  const boost::shared_ptr<EventStreamInterface> &stream_interface);
    ~EventListener();
    
    void startListener();
    void killListener();
    
private:
    void service();
    
    EventBufferReader reader;
    const boost::shared_ptr<EventStreamInterface> handler;
    std::thread thread;
    static_assert(ATOMIC_BOOL_LOCK_FREE == 2, "std::atomic_bool is not always lock-free");
    std::atomic_bool servicing;
    
};


END_NAMESPACE_MW


#endif


























