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

//#include "MonkeyWorksCore/GenericEventFunctor.h"
#include "MWCocoaEvent.h"
#include "MonkeyWorksCore/EventStreamInterface.h"
#include <boost/shared_ptr.hpp>
using namespace std;
using namespace boost;


namespace mw {
    
    
    EventCallback create_cocoa_event_callback(id _receiver, SEL _selector, id _syncobject=Nil);
    EventCallback create_bindings_bridge_event_callback(id _receiver, NSString *_bindings_key);
    
    
    
	// derived template class
	class CocoaEventFunctor
		{
		private:
			id receiver;
			SEL selector;
			id syncobject;
		public:
			
			// constructor - takes pointer to an object and pointer to a member and stores
			// them in two private variables
			CocoaEventFunctor(id _receiver, SEL _selector, id _syncobject=Nil)
			{
        
				receiver = _receiver;
				selector = _selector;
                syncobject = _syncobject;
        
			};
			
			
			// override operator "()"
			virtual void operator()(const shared_ptr<Event> &event)
			{ 
				NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
				//objc_registerThreadWithCollector();
				Datum data(event->getData());
				id cocoaEvent = [[[MWCocoaEvent alloc] initWithData:&data 
															andCode:event->getEventCode() 
															andTime:event->getTime()] autorelease];
				
        //@synchronized(syncobject){
          if([receiver respondsToSelector:selector]) {
            // DDC: 5/08 changed to "OnMainThread"
            // this is to ease development of plugins so that Cocoa drawing calls can be made here
            [receiver performSelectorOnMainThread:selector withObject:cocoaEvent waitUntilDone:YES];
            //[receiver performSelector:selector withObject:cocoaEvent];
          } else {
            NSString *sn = NSStringFromSelector(selector);
            NSString *errorMessage = @"Cannot call selector from specified receiver: ";
            merror(M_CLIENT_MESSAGE_DOMAIN, [[errorMessage stringByAppendingString:sn] cStringUsingEncoding:NSASCIIStringEncoding]);
          }
        //}
				[pool release];
			};         
		};
	
	
	// derived template class
	class CocoaBindingsBridgeFunctor
		{
		private:
			id receiver;
			NSString *bindings_key;
			
		public:
			
			// constructor - takes pointer to an object and pointer to a member and stores
			// them in two private variables
			CocoaBindingsBridgeFunctor(id _receiver, NSString *_bindings_key)
			{
				receiver = _receiver;
				bindings_key = _bindings_key;
			};
			
			
			// override operator "()"
			virtual void operator()(const shared_ptr<Event> &event)
			{ 
				
				Datum data(event->getData());
				
				NSNumber *data_number = [NSNumber numberWithDouble:(double)data];
				[receiver setValue:data_number forKey:bindings_key];
			};         
		};
	
}
#endif

