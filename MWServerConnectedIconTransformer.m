//
//  ServerConnectedIconTransformer.m
//  New Client
//
//  Created by David Cox on 3/11/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import "MWServerConnectedIconTransformer.h"

#define CONNECTED_IMAGE @"OkCheck"
#define DISCONNECTED_IMAGE @"NoX"

//#define CONNECTED_IMAGE @"OkCheck"
//#define DISCONNECTED_IMAGE @"NotConnected"


@implementation MWServerConnectedIconTransformer

+ (Class)transformedValueClass { return [NSImage class]; }
+ (BOOL)allowsReverseTransformation { return NO; }
- (id)transformedValue:(id)value {	
	
	NSString *imageName;
	
	if([value respondsToSelector:@selector(boolValue)] && [value boolValue]){
			
		imageName = [[NSBundle mainBundle] pathForResource:CONNECTED_IMAGE ofType:@"tiff"];
		NSImage* connected_icon = [[NSImage alloc] initWithContentsOfFile:imageName];
		return connected_icon;
		//return imageName;

	}
	
	//imageName = [[NSBundle mainBundle] pathForResource:@"OkCheck" ofType:@"tiff"];
	imageName = [[NSBundle mainBundle] pathForResource:DISCONNECTED_IMAGE ofType:@"tiff"];
	NSImage* disconnected_icon = [[NSImage alloc] initWithContentsOfFile:imageName];
	return disconnected_icon;
	//return imageName;
}

@end
