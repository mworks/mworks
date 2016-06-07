//
//  MWClientProtocol.h
//  MWorksCocoa
//
//  Created by David Cox on 12/21/09.
//  Copyright 2009 Harvard University. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#import <MWorksCore/Client.h>
#import <MWorksCore/GenericData.h>

#import "MWNotebook.h"
#import "MWClientServerBase.h"


@class MWCodec;  // Forward declaration


@protocol MWClientProtocol <MWClientServerBase>

- (shared_ptr<mw::Client>) coreClient;

- (void)registerEventCallbackWithReceiver:(id)receiver
                                 selector:(SEL)selector
                              callbackKey:(const char *)key
							  forVariable:(NSString *)tag
                             onMainThread:(BOOL)on_main;

- (void)registerBindingsBridgeWithReceiver:(id)receiver 
                               bindingsKey:(NSString *)bindings_key
                               callbackKey:(const char *)key
							   forVariable:(NSString *)tag;

- (void)unregisterCallbacksWithKey:(const char *)key locking:(BOOL)locking;

- (void)updateVariableWithCode:(int)code withData:(mw::Datum *)data;
- (void)updateVariableWithTag:(NSString *)tag withData:(mw::Datum *)data;

- (MWNotebook *) notebook;
- (NSDictionary *)varGroups;
- (MWCodec *)variables;

- (NSWindow *)groupedPluginWindow;

@end


@protocol MWClientPluginWorkspaceState <NSObject>

- (NSDictionary *)workspaceState;
- (void)setWorkspaceState:(NSDictionary *)workspaceState;

@end



























