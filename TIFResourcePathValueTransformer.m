//
//  TIFResourcePathValueTransformer.m
//  NewEditor
//
//  Created by David Cox on 11/4/07.
//  Copyright 2007 __MyCompanyName__. All rights reserved.
//

#import "TIFResourcePathValueTransformer.h"


@implementation TIFResourcePathValueTransformer

+ (Class)transformedValueClass { return [NSString class]; }

+ (BOOL)allowsReverseTransformation{
	return NO;
}

- (NSString *)transformedValue:(NSString *)name{
	NSString* fullpath = [[NSBundle mainBundle] pathForResource:name ofType:@"tif"];
	return fullpath;
}

@end
