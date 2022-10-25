//
//  MWCodec.h
//  New Client
//
//  Created by David Cox on 3/14/08.
//  Copyright 2008 Harvard University. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#include <MWorksCore/Client.h>

#import "MWCocoaEvent.h"
#import "MWClientProtocol.h"


@interface MWCodec : NSObject {

    shared_ptr<mw::Client> core;
	// The client instance owns the codec instance, so use a weak reference here
	id<MWClientProtocol> __weak clientInstance;
	
	NSMutableArray *variable_names;
	NSMutableArray *variable_codes;
	
	NSMutableArray *variable_changed;
	
	NSTimer *update_timer;
	
}

- (NSMutableArray *)variableNames;
- (int)countOfVariableNames;
- (id) objectInVariableNamesAtIndex: (int)i;

- (id) initWithClientInstance: (id<MWClientProtocol>) _client;

// Update internally stored information given the arrival of a 
// new codec.
- (void)receiveCodec:(MWCocoaEvent *)event;

// KVC compliance (keys are variable names
- (id) valueForKey: (NSString *)key;
- (void) setValue: (id)val forKey: (NSString *)key;
- (id) valueForUndefinedKey: (NSString *)key;

- (void)updateChangedValues;

- (mw::Datum)valueForVariable:(NSString *)name;
- (void)setValue:(const mw::Datum &)val forVariable:(NSString *)name;

- (NSString *)descriptionForVariable:(NSString *)name;


@end
