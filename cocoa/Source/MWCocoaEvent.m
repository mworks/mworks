//
//  MWCocoaEvent.m
//  MWorksCocoa
//
//  Created by Ben Kennedy on 8/14/07.
//  Copyright 2007 __MyCompanyName__. All rights reserved.
//

#import "MWCocoaEvent.h"
#import "MWCocoaEventFunctor.h"

@implementation MWCocoaEvent

- (id) initWithData:(Datum *)data 
			andCode:(int)code 
			andTime:(MWorksTime)time {
	self = [super init];
	if (self != nil) {
		_data = new Datum(*data);
		_code = code;
		_time = time;
	}
	return self;
}

- (void)dealloc {
    delete _data;
    [super dealloc];
}

// finalize is called instead of dealloc when garbage collection is enabled.  Since we don't currently
// support GC, there's no need for this.
/*
- (void)finalize {
    delete _data;
    [super finalize];
}
*/

- (MWorksTime)time {
	return _time;
}

- (Datum *)data {
	return _data;
}

- (int)code {
	return _code;
}

@end
