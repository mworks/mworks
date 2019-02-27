//
//  MWURLStringValueTransformer.m
//  MWClient
//
//  Created by David Cox on 2/24/09.
//  Copyright 2009 Harvard University. All rights reserved.
//

#import "MWURLStringValueTransformer.h"


@implementation MWURLStringValueTransformer

+ (Class)transformedValueClass { return [NSURL class]; }
+ (BOOL)allowsReverseTransformation { return YES; }
- (id)transformedValue:(id)str {
	if(str == Nil){
        return Nil;
    }
    
	return [NSURL fileURLWithPath:str];
}

- (id)reverseTransformedValue:(id)url {
    if(url == Nil){
        return Nil;
    }
    return [url absoluteString];
}


@end
