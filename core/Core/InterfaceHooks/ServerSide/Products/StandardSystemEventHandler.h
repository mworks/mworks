/*!
* @header StandardSystemEventHandler.h
 *
 * @discussion Provides standardized handling of incoming control events
 *             e.g. from an external network connected client
 *
 */

#ifndef _DEFAULT_EVENT_HANDLER_H__
#define _DEFAULT_EVENT_HANDLER_H__

#include "EventStreamInterface.h"


BEGIN_NAMESPACE_MW


class StandardSystemEventHandler : public EventStreamInterface {
    
public:
    StandardSystemEventHandler();
    
    void handleEvent(boost::shared_ptr<Event> event) override;
    void putEvent(boost::shared_ptr<Event> event) override { }
    
private:
    void handleSystemEvent(const Datum &sysEvent);
    
};


END_NAMESPACE_MW


#endif /* _DEFAULT_EVENT_HANDLER_H__ */
