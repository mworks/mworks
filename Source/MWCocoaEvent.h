//
//  MWCocoaEvent.h
//  MWorksCocoa
//
//  Created by Ben Kennedy on 8/14/07.
//  Copyright 2007 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "MWorksCore/GenericData.h"
using namespace mw;

@interface MWCocoaEvent : NSObject {
	Datum *_data;
	int _code;
	MWorksTime _time;
}

- (id) initWithData:(Datum *)data 
			andCode:(int)code 
			andTime:(MWorksTime)time;


- (MWorksTime)time;	
- (Datum *)data;
- (int)code;


@end
