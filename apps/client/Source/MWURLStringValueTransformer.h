//
//  MWURLStringValueTransformer.h
//  MWClient
//
//  Created by David Cox on 2/24/09.
//  Copyright 2009 Harvard University. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface MWURLStringValueTransformer : NSValueTransformer {}

+ (Class)transformedValueClass;
+ (BOOL)allowsReverseTransformation;
- (id)transformedValue:(id)value;

@end
