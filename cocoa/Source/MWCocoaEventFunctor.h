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

//#include "MWorksCore/GenericEventFunctor.h"
#include "MWCocoaEvent.h"
#import "CocoaBindingsSetter.h"
#include "MWorksCore/EventStreamInterface.h"
#include <boost/shared_ptr.hpp>


namespace mw {
    
    
    EventCallback create_cocoa_event_callback(id _receiver, SEL _selector, bool on_main=true, id _syncobject=Nil);
    EventCallback create_bindings_bridge_event_callback(id _receiver, NSString *_bindings_key);
    
    
    
	// derived template class
	class CocoaEventFunctor {
		private:
			id receiver;
			SEL selector;
			id syncobject;
        bool on_main_thread;
		public:
			
			// constructor - takes pointer to an object and pointer to a member and stores
			// them in two private variables
			CocoaEventFunctor(id _receiver, SEL _selector, bool _on_main_thread = true, id _syncobject=Nil)
			{
        
                on_main_thread = _on_main_thread;
				receiver = _receiver;
				selector = _selector;
                syncobject = _syncobject;
        
			};
			
        
            virtual ~CocoaEventFunctor() { }
			
			// override operator "()"
			virtual void operator()(const shared_ptr<Event> &event)
			{ 
				NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
				
				Datum data(event->getData());
				MWCocoaEvent *cocoaEvent = [[MWCocoaEvent alloc] initWithData:&data 
                                                                      andCode:event->getEventCode() 
                                                                      andTime:event->getTime()];
				
        //@synchronized(syncobject){
          if([receiver respondsToSelector:selector]) {
            // DDC: 5/08 changed to "OnMainThread"
            // this is to ease development of plugins so that Cocoa drawing calls can be made here
              // Event handling refactor: changed wailUntilDone to NO
            
              if(on_main_thread){
                  [receiver performSelectorOnMainThread:selector withObject:cocoaEvent waitUntilDone:NO];
              } else {
                  [receiver performSelector:selector withObject:cocoaEvent];
              }
            //[receiver performSelector:selector withObject:cocoaEvent];
          } else {
            NSString *sn = NSStringFromSelector(selector);
            NSString *errorMessage = @"Cannot call selector from specified receiver: ";
            merror(M_CLIENT_MESSAGE_DOMAIN, [[errorMessage stringByAppendingString:sn] cStringUsingEncoding:NSASCIIStringEncoding]);
          }
        //}
                [cocoaEvent release];
				[pool drain];
			};         
		};
	
	
	// derived template class
	class CocoaBindingsBridgeFunctor
		{
		private:
            CocoaBindingsSetter *setter;
			
		public:
			
			// constructor - takes pointer to an object and pointer to a member and stores
			// them in two private variables
			CocoaBindingsBridgeFunctor(id _receiver, NSString *_bindings_key)
			{
                setter = [[CocoaBindingsSetter alloc] initWithReceiver:_receiver bindingsKey:_bindings_key];                
			}
			
			virtual ~CocoaBindingsBridgeFunctor(){ }
            
			// override operator "()"
			virtual void operator()(const shared_ptr<Event> &event)
			{ 
                Datum data(event->getData());
                // We can't use numberWithDouble: here, because there may not be an autorelease pool in place
				NSNumber *data_number = [[NSNumber alloc] initWithDouble:(double)data];
				[setter performSelectorOnMainThread:@selector(setValue:) withObject:data_number waitUntilDone:NO];
                [data_number release];
			}         
		};
	
}
#endif

