//
//  MarionetteEvent.m
//  MWorksCore
//
//  Created by bkennedy on 8/25/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import "MarionetteEvent.h"


@implementation MarionetteEvent

- (id)initWithVariableName:(NSString *)var_name 
				   andData:(Datum *)_data 
   andProcessDataAfterward:(BOOL)_continue_processing {
	self = [super init];
	if(self != nil) {
		variable = [var_name copy];
		if(_data != 0) {
			data = new Datum(*_data);
		} else {
			data = new Datum();
		}
		
		continue_processing = _continue_processing;
	}
	return self;
}

+ (id)eventWithVariableName:(NSString *)var_name 
					andData:(Datum *)_data 
	andProcessDataAfterward:(BOOL)_continue_processing {
 	return [[[self alloc] initWithVariableName:var_name
									   andData:_data 
					   andProcessDataAfterward:_continue_processing] autorelease];

}

- (void)dealloc {
	[variable release];
	delete data;
	[super dealloc];
}

- (BOOL)continueProcessing {return continue_processing;}
- (NSString *)variable {return variable;}
- (Datum *)data { return data; }

- (BOOL)matches:(Datum *)data_to_match {
	// if the data is undefined, then match any data that comes in
	if(data->isUndefined()) {
		return YES;
	} else {
	 Datum result(*data == *data_to_match);
		return result.getBool();
	}
}



@end
