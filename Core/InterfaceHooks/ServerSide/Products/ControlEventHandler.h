/*!
* @header ControlEventHandler.h
 *
 * @discussion Provides standardized handling of incoming control events
 *             e.g. from an external network connected client
 *
 */


#ifndef _DEFAULT_EVENT_HANDLER_H__
#define _DEFAULT_EVENT_HANDLER_H__

#include "EventStreamInterface.h"
namespace mw {
    class ControlEventHandler : public EventStreamInterface {
        
    public:
        ControlEventHandler();
        virtual ~ControlEventHandler() {};
        virtual void handleEvent(shared_ptr<Event> event);
        virtual void putEvent(shared_ptr<Event> event){ }
    protected:
        virtual void handleSystemEvent(const Datum &sysEvent);
        

    };
}
#endif

