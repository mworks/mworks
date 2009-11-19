//
//  MarionetteEvent.h
//  MonkeyWorksCore
//
//  Created by bkennedy on 8/25/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "MonkeyWorksCore/GenericData.h"
using namespace mw;

@interface MarionetteEvent : NSObject {
	BOOL continue_processing;
	NSString *variable;
 Datum *data;
}

- (id)initWithVariableName:(NSString *)var_name 
				   andData:(Datum *)_data 
   andProcessDataAfterward:(BOOL)_continue_processing;

+ (id)eventWithVariableName:(NSString *)var_name 
					andData:(Datum *)_data 
	andProcessDataAfterward:(BOOL)_continue_processing;

- (BOOL)continueProcessing;
- (NSString *)variable;
- (Datum *)data;

- (BOOL)matches:(Datum *)data_to_match;
@end
