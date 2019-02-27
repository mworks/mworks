//
//  MWErrorColorTransformer.m
//  New Client
//
//  Created by David Cox on 3/23/08.
//  Copyright 2008 Harvard University. All rights reserved.
//

#import "MWErrorColorTransformer.h"


@implementation MWErrorColorTransformer

+ (Class)transformedValueClass { return [NSColor class]; }
+ (BOOL)allowsReverseTransformation { return NO; }

- (id)transformedValue:(id)value {	
	
	NSColor *color;
	
	if([value boolValue]){
        color = [NSColor systemRedColor];
	} else {
        color = [NSColor labelColor];
	}
	return color;
}
@end
