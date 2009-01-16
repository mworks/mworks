/*!
* @header DefaultEventHandler.h
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

#include "EventHandler.h"
namespace mw {
class DefaultEventHandler : public EventHandler {
	
public:
	virtual ~DefaultEventHandler() {};
	virtual void handleEvent(shared_ptr<Event> &event);
protected:
        virtual void handleSystemEvent(const Data &sysEvent);
	
};
}
#endif

