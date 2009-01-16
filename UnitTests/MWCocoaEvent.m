//
//  MWCocoaEvent.m
//  MonkeyWorksCocoa
//
//  Created by Ben Kennedy on 8/14/07.
//  Copyright 2007 __MyCompanyName__. All rights reserved.
//

#import "MWCocoaEvent.h"


@implementation MWCocoaEvent

- (id) initWithData:(Data *)data 
			andCode:(int)code 
			andTime:(MonkeyWorksTime)time {
	self = [super init];
	if (self != nil) {
		_data = new Data(*data);
		_code = code;
		_time = time;
	}
	return self;
}

- (void) dealloc {
	delete _data;
	[super dealloc];
}

- (MonkeyWorksTime)time {
	return _time;
}

- (Data *)data {
	return _data;
}

- (int)code {
	return _code;
}

@end
