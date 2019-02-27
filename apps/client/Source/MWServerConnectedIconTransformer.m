//
//  ServerConnectedIconTransformer.m
//  New Client
//
//  Created by David Cox on 3/11/08.
//  Copyright 2008 Harvard University. All rights reserved.
//

#import "MWServerConnectedIconTransformer.h"


@implementation MWServerConnectedIconTransformer

+ (Class)transformedValueClass { return [NSImage class]; }
+ (BOOL)allowsReverseTransformation { return NO; }
- (id)transformedValue:(id)value {	
	
	NSString *imageName;
	
	if([value respondsToSelector:@selector(boolValue)] && [value boolValue]){
		imageName = [[NSBundle mainBundle] pathForResource:@"success" ofType:@"png"];
		NSImage* connected_icon = [[NSImage alloc] initWithContentsOfFile:imageName];
		return connected_icon;
	}
	
	imageName = [[NSBundle mainBundle] pathForResource:@"error" ofType:@"png"];
	NSImage* disconnected_icon = [[NSImage alloc] initWithContentsOfFile:imageName];
	return disconnected_icon;
}

@end
