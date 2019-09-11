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

- (void)updateVariableWithCode:(int)code withData:(mw::Datum *)data;
- (void)updateVariableWithTag:(NSString *)tag withData:(mw::Datum *)data;

- (MWNotebook *) notebook;
- (NSDictionary *)varGroups;
- (MWCodec *)variables;
- (NSArray *)variableNames;

- (NSWindow *)groupedPluginWindow;

@end


@protocol MWClientPluginWindowController <NSObject>

- (void)setInGroupedWindow:(BOOL)isInGroupedWindow;

@end


@protocol MWClientPluginWorkspaceState <NSObject>

- (NSDictionary *)workspaceState;
- (void)setWorkspaceState:(NSDictionary *)workspaceState;

@end



























