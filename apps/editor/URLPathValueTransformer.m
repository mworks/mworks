//
//  URLPathValueTransformer.m
//  NewEditor
//
//  Created by David Cox on 7/22/08.
//  Copyright 2008 The Rowland Institute at Harvard. All rights reserved.
//

#import "URLPathValueTransformer.h"


@implementation URLPathValueTransformer


+ (Class)transformedValueClass { return [NSURL class]; }

+ (BOOL)allowsReverseTransformation{
	return YES;
}

- (NSURL *)transformedValue:(NSString *)path{
    if(path == Nil){
        return [NSURL fileURLWithPath:@"file://localhost"];
    }
	return [NSURL fileURLWithPath:path]; 
}

- (NSString *)reverseTransformedValue:(NSURL *)path{
	return [path relativePath];
}

@end
