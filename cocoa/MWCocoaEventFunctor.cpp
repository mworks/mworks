/*
 *  MWCocoaEventFunctor.cpp
 *  MWorksCocoa
 *
 *  Created by David Cox on 11/2/09.
 *  Copyright 2009 Harvard University. All rights reserved.
 *
 */

#import "MWCocoaEventFunctor.h"


namespace mw{

EventCallback create_cocoa_event_callback(id _receiver, SEL _selector, bool on_main){

    shared_ptr<CocoaEventFunctor> functor_object(new CocoaEventFunctor(_receiver, _selector, on_main));
    return boost::bind(&CocoaEventFunctor::operator(), functor_object, _1);
}


EventCallback create_bindings_bridge_event_callback(id _receiver, NSString *_bindings_key){
    shared_ptr<CocoaBindingsBridgeFunctor> functor_object(new CocoaBindingsBridgeFunctor(_receiver, _bindings_key));
    return boost::bind(&CocoaBindingsBridgeFunctor::operator(), functor_object, _1);
    
}
    
}

