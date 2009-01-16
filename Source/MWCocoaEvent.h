//
//  MWCocoaEvent.h
//  MonkeyWorksCocoa
//
//  Created by Ben Kennedy on 8/14/07.
//  Copyright 2007 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "MonkeyWorksCore/GenericData.h"
using namespace mw;

@interface MWCocoaEvent : NSObject {
	Data *_data;
	int _code;
	MonkeyWorksTime _time;
}

- (id) initWithData:(Data *)data 
			andCode:(int)code 
			andTime:(MonkeyWorksTime)time;

- (void) dealloc;

- (MonkeyWorksTime)time;	
- (Data *)data;
- (int)code;


@end
