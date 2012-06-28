//
//  MWClientProtocol.h
//  MWorksCocoa
//
//  Created by David Cox on 12/21/09.
//  Copyright 2009 Harvard University. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#import "MWorksCore/GenericData.h"
#import "MWNotebook.h"


@protocol MWClientProtocol <NSObject>

// Methods for registering to be notified of events from the core object
- (void)registerEventCallbackWithReceiver:(id)receiver 
                                 selector:(SEL)selector
                              callbackKey:(const char *)key
                             onMainThread:(BOOL)on_main;

- (void)registerEventCallbackWithReceiver:(id)receiver 
                                 selector:(SEL)selector
                              callbackKey:(const char *)key
						  forVariableCode:(int)code
                             onMainThread:(BOOL)on_main;

- (void)registerEventCallbackWithReceiver:(id)receiver 
                                 selector:(SEL)selector
                              callbackKey:(const char *)key
							  forVariable:(NSString *)tag
                             onMainThread:(BOOL)on_main;

- (void)registerBindingsBridgeWithReceiver:(id)receiver 
                               bindingsKey:(NSString *)bindings_key
                               callbackKey:(const char *)key
							   forVariable:(NSString *)tag;

- (void)unregisterCallbacksWithKey:(const char *)key;
- (void)unregisterCallbacksWithKey:(const char *)key locking:(BOOL)locking;

- (NSNumber *)codeForTag:(NSString *)tag;
- (void)updateVariableWithCode:(int)code withData:(mw::Datum *)data;
- (void)updateVariableWithTag:(NSString *)tag withData:(mw::Datum *)data;

- (MWNotebook *) notebook;
- (NSDictionary *)varGroups;

@end
