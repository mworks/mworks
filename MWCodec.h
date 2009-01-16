//
//  MWCodec.h
//  New Client
//
//  Created by David Cox on 3/14/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <MonkeyWorksCore/Client.h>
#import <MonkeyWorksCocoa/MWCocoaEvent.h>

#define MW_CODEC_CALLBACK_KEY	"mw_codec_callback_key"
#define MW_CODEC_VALUE_CHANGE_CALLBACK_KEY	"mw_codec_value_change_callback_key"

@interface MWCodec : NSObject {

	shared_ptr<mw::Client> core;
	id clientInstance;
	
	NSMutableArray *variable_names;
	NSMutableArray *variable_codes;
	
	NSMutableArray *variable_changed;
	
	NSTimer *update_timer;
	
}

- (NSMutableArray *)variableNames;
- (int)countOfVariableNames;
- (id) objectInVariableNamesAtIndex: (int)i;

- (id) initWithClientInstance: (id) _client;

// Update internally stored information given the arrival of a 
// new codec.
- (void)receiveCodec:(MWCocoaEvent *)event;

// KVC compliance (keys are variable names
- (id) valueForKey: (NSString *)key;
- (void) setValue: (id)val forKey: (NSString *)key;
- (id) valueForUndefinedKey: (NSString *)key;

// Handle events coming from the core client object and issue
// KVC compliant notifications of changing values
- (void)handleCallbackEvent:(MWCocoaEvent *)event;
- (void)updateChangedValues;


@end
