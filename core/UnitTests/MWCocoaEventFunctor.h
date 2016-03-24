/*
 *  MWCocoaEventFunctor.h
 *  MWorksCocoa
 *
 *  Created by bkennedy on 9/6/07.
 *  Copyright 2007 MIT. All rights reserved.
 *
 */

#ifndef __COCOA_EVENT_FUNCTOR_H_
#define __COCOA_EVENT_FUNCTOR_H_

#include "MWCocoaEvent.h"


BEGIN_NAMESPACE_MW


// derived template class
class CocoaEventFunctor
{
private:
	id receiver;
	SEL selector;
	
public:
		
	// constructor - takes pointer to an object and pointer to a member and stores
	// them in two private variables
	CocoaEventFunctor(id _receiver, SEL _selector, const std::string &key)
    {
			receiver = _receiver;
			selector = _selector;
    };
	
	
	// override operator "()"
    void operator()(const shared_ptr<Event> &event)
	{ 
		@autoreleasepool {
		
 	 Datum data(event->getData());
		id cocoaEvent = [[MWCocoaEvent alloc] initWithData:&data 
													andCode:event->getEventCode() 
													andTime:event->getTime()];
		
		if([receiver respondsToSelector:selector]) {
			// DDC: 5/08 changed to "OnMainThread"
			[receiver performSelectorOnMainThread:selector withObject:cocoaEvent waitUntilDone:YES];
			//[receiver performSelector:selector withObject:cocoaEvent];
		} else {
			NSString *sn = NSStringFromSelector(selector);
			NSString *errorMessage = @"Cannot call selector from specified receiver: ";
			merror(M_CLIENT_MESSAGE_DOMAIN, "%s", [[errorMessage stringByAppendingString:sn] cStringUsingEncoding:NSASCIIStringEncoding]);
		}
		
		}
	};         
};


END_NAMESPACE_MW


#endif

