/*!
* @header DefaultEventStreamInterface.h
 *
 * @discussion 
 *
 * History:<BR/>
 * paul on 1/27/06 - Created.<BR/>
 * 
 * @copyright MIT
 * @updated 1/27/06
 * @version 1.0.0
 */

#ifndef _DEFAULT_EVENT_HANDLER_H__
#define _DEFAULT_EVENT_HANDLER_H__

#include "EventStreamInterface.h"
namespace mw {
class DefaultEventStreamInterface : public EventStreamInterface {
	
public:
    DefaultEventStreamInterface();
	virtual ~DefaultEventStreamInterface() {};
	virtual void handleEvent(shared_ptr<Event> event);
    virtual void putEvent(shared_ptr<Event> event){ }
protected:
        virtual void handleSystemEvent(const Datum &sysEvent);
    
	
};
}
#endif

