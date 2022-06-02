//
//  MWClientProtocol.h
//  MWorksCocoa
//
//  Created by David Cox on 12/21/09.
//  Copyright 2009 Harvard University. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#include <MWorksCore/Client.h>
#include <MWorksCore/GenericData.h>

#import <MWorksCocoa/MWClientPlugin.h>  // Include for compatibility with existing code
#import <MWorksCocoa/MWClientServerBase.h>
#import <MWorksCocoa/MWNotebook.h>


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
