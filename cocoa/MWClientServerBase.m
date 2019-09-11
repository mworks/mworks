//
//  MWClientServerBase.m
//  MWorksCocoa
//
//  Created by Christopher Stawarz on 9/11/19.
//

#import "MWClientServerBase.h"

#import <MWorksSwift/MWorksSwiftErrors_Private.h>
#import <MWorksSwift/MWKCore_Private.h>

#import "MWCocoaEventFunctor.h"


@implementation MWClientServerBase {
    MWKCore *_core;
}


- (instancetype)initWithCore:(MWKCore *)core {
    self = [super init];
    if (self) {
        _core = core;
    }
    return self;
}


- (MWKCore *)core {
    return _core;
}


- (NSNumber *)codeForTag:(NSString *)tag {
    return @([self.core codeForTag:tag]);
}


- (void)registerEventCallbackWithReceiver:(id)receiver
                                 selector:(SEL)selector
                              callbackKey:(const char *)key
                             onMainThread:(BOOL)on_main
{
    try {
        self.core.core->registerCallback(create_cocoa_event_callback(receiver, selector, on_main), key);
    } catch (...) {
        MWorksSwiftLogException(std::current_exception());
    }
}


- (void)registerEventCallbackWithReceiver:(id)receiver
                                 selector:(SEL)selector
                              callbackKey:(const char *)key
                          forVariableCode:(int)code
                             onMainThread:(BOOL)on_main
{
    try {
        if (code >= 0) {
            self.core.core->registerCallback(code, create_cocoa_event_callback(receiver, selector, on_main), key);
        }
    } catch (...) {
        MWorksSwiftLogException(std::current_exception());
    }
}


- (void)registerEventCallbackWithReceiver:(id)receiver
                                 selector:(SEL)selector
                              callbackKey:(const char *)key
                              forVariable:(NSString *)tag
                             onMainThread:(BOOL)on_main
{
    try {
        self.core.core->registerCallback(tag.UTF8String, create_cocoa_event_callback(receiver, selector, on_main), key);
    } catch (...) {
        MWorksSwiftLogException(std::current_exception());
    }
}


// receiver: the object that will receive the KVC messages
// bindingsKey: the key (in the bindings sense of the word) in question
// callbackKey: a unique string to identify this particular callback
// forVariable: the tag name of the MWorks variable in question
- (void)registerBindingsBridgeWithReceiver:(id)receiver
                               bindingsKey:(NSString *)bindings_key
                               callbackKey:(const char *)key
                               forVariable:(NSString *)tag
{
    try {
        self.core.core->registerCallback(tag.UTF8String,
                                         create_bindings_bridge_event_callback(receiver, bindings_key),
                                         key);
    } catch (...) {
        MWorksSwiftLogException(std::current_exception());
    }
}


- (void)unregisterCallbacksWithKey:(const char *)key {
    [self.core unregisterCallbacksWithKey:@(key)];
}


@end
