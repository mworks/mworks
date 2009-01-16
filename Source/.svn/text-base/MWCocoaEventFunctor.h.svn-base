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
#include <boost/shared_ptr.hpp>
using namespace std;
using namespace boost;


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
	
	
	// derived template class
	class CocoaBindingsBridgeFunctor : public GenericEventFunctor
		{
		private:
			id receiver;
			NSString *bindings_key;
			
		public:
			
			// constructor - takes pointer to an object and pointer to a member and stores
			// them in two private variables
			CocoaBindingsBridgeFunctor(id _receiver, NSString *_bindings_key, const std::string &key) : GenericEventFunctor (key)
			{
				receiver = _receiver;
				bindings_key = _bindings_key;
			};
			
			
			// override operator "()"
			virtual void operator()(const shared_ptr<Event> &event)
			{ 
//				NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
				
				Data data(event->getData());
//				id cocoaEvent = [[[MWCocoaEvent alloc] initWithData:&data 
//															andCode:event->getEventCode() 
//															andTime:event->getTime()] autorelease];
				
				
				
				NSNumber *data_number = [NSNumber numberWithDouble:(double)data];
				[receiver setValue:data_number forKey:bindings_key];
				//if([receiver respondsToSelector:selector]) {
				// DDC: 5/08 changed to "OnMainThread"
				
				//[receiver performSelectorOnMainThread:selector withObject:cocoaEvent waitUntilDone:YES];
				/*} else {
				 NSString *sn = NSStringFromSelector(selector);
				 NSString *errorMessage = @"Cannot call selector from specified receiver: ";
				 merror(M_CLIENT_MESSAGE_DOMAIN, [[errorMessage stringByAppendingString:sn] cStringUsingEncoding:NSASCIIStringEncoding]);
				 }*/
				
				//	[pool release];
			};         
		};
	
}
#endif

