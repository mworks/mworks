//
//  EscapedStringValueTransformer.m
//  NewEditor
//
//  Created by David Cox on 12/4/07.
//  Copyright 2007 __MyCompanyName__. All rights reserved.
//

#import "EscapedStringValueTransformer.h"


@implementation EscapedStringValueTransformer

+ (Class)transformedValueClass { return [NSString class]; }

+ (BOOL)allowsReverseTransformation{
	return YES;
}

- (NSString *)transformedValue:(NSString *)name{
	return [name stringByReplacingOccurrencesOfString:@"_" withString:@" "];
}

- (NSString *)reverseTransformedValue:(NSString *)name{
	return [name stringByReplacingOccurrencesOfString:@" " withString:@"_"];
}




@end
