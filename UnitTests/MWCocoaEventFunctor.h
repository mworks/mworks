/*
 *  MWCocoaEventFunctor.h
 *  MonkeyWorksCocoa
 *
 *  Created by bkennedy on 9/6/07.
 *  Copyright 2007 MIT. All rights reserved.
 *
 */

#ifndef __COCOA_EVENT_FUNCTOR_H_
#define __COCOA_EVENT_FUNCTOR_H_

#include "MonkeyWorksCore/GenericEventFunctor.h"
#include "MWCocoaEvent.h"
namespace mw {
// derived template class
class CocoaEventFunctor : public GenericEventFunctor
{
private:
	id receiver;
	SEL selector;
	
public:
		
	// constructor - takes pointer to an object and pointer to a member and stores
	// them in two private variables
	CocoaEventFunctor(id _receiver, SEL _selector, const std::string &key) : GenericEventFunctor (key)
    {
			receiver = _receiver;
			selector = _selector;
    };
	
	
	// override operator "()"
	virtual void operator()(const shared_ptr<Event> &event)
	{ 
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
		
		Data data(event->getData());
		id cocoaEvent = [[[MWCocoaEvent alloc] initWithData:&data 
													andCode:event->getEventCode() 
													andTime:event->getTime()] autorelease];
		
		if([receiver respondsToSelector:selector]) {
			// DDC: 5/08 changed to "OnMainThread"
			[receiver performSelectorOnMainThread:selector withObject:cocoaEvent waitUntilDone:YES];
			//[receiver performSelector:selector withObject:cocoaEvent];
		} else {
			NSString *sn = NSStringFromSelector(selector);
			NSString *errorMessage = @"Cannot call selector from specified receiver: ";
			merror(M_CLIENT_MESSAGE_DOMAIN, [[errorMessage stringByAppendingString:sn] cStringUsingEncoding:NSASCIIStringEncoding]);
		}
		
		[pool release];
	};         
};
}
#endif

