//
//  RedIfNotNilValueTransformer.m
//  NewEditor
//
//  Created by David Cox on 11/14/07.
//  Copyright 2007 __MyCompanyName__. All rights reserved.
//

#import "RedIfNotNilValueTransformer.h"


@implementation RedIfNotNilValueTransformer

+ (Class)transformedValueClass { return [NSColor class]; }

+ (BOOL)allowsReverseTransformation{
	return NO;
}

- (NSColor *)transformedValue:(id)obj{

	if(obj != Nil){
		return [NSColor colorWithDeviceRed:1.0 green:0.0 blue:0.0 alpha:1.0];
	} else {
		return [NSColor colorWithDeviceRed:0.0 green:0.0 blue:0.0 alpha:1.0];
	}
}


@end
