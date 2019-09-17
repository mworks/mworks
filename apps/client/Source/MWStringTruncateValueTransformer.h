//
//  MWStringTruncateValueTransformer.h
//  New Client
//
//  Created by David Cox on 3/12/08.
//  Copyright 2008 Harvard University. All rights reserved.
//

@import Cocoa;

#define TRUNC_MAX_CHARS	30
#define TRUNC_BEGIN_CHARS	10
#define TRUNC_END_CHARS		20

@interface MWStringTruncateValueTransformer : NSValueTransformer {

}


+ (Class)transformedValueClass;
+ (BOOL)allowsReverseTransformation;
- (id)transformedValue:(id)value;


@end
