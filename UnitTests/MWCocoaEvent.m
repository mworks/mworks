//
//  MWCocoaEvent.m
//  MWorksCocoa
//
//  Created by Ben Kennedy on 8/14/07.
//  Copyright 2007 __MyCompanyName__. All rights reserved.
//

#import "MWCocoaEvent.h"


@implementation MWCocoaEvent

- (id) initWithData:(Datum *)data 
			andCode:(int)code 
			andTime:(MWTime)time {
	self = [super init];
	if (self != nil) {
		_data = new Datum(*data);
		_code = code;
		_time = time;
	}
	return self;
}

- (void) dealloc {
	delete _data;
	[super dealloc];
}

- (MWTime)time {
	return _time;
}

- (Datum *)data {
	return _data;
}

- (int)code {
	return _code;
}

@end
