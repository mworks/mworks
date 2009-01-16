//
//  MWErrorColorTransformer.m
//  New Client
//
//  Created by David Cox on 3/23/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import "MWErrorColorTransformer.h"


@implementation MWErrorColorTransformer

+ (Class)transformedValueClass { return [NSColor class]; }
+ (BOOL)allowsReverseTransformation { return NO; }

- (id)transformedValue:(id)value {	
	
	NSColor *color;
	
	if([value boolValue]){
		color = [NSColor colorWithDeviceRed:(CGFloat)0.5 green:(CGFloat)0.0 blue:(CGFloat)0.0 alpha:(CGFloat)1.0];
	} else {
		color = [NSColor colorWithDeviceRed:(CGFloat)0.0 green:(CGFloat)0.0 blue:(CGFloat)0.0 alpha:(CGFloat)1.0];
	}
	return color;
}
@end
