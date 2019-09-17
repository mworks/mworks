//
//  MWErrorColorTransformer.h
//  New Client
//
//  Created by David Cox on 3/23/08.
//  Copyright 2008 Harvard University. All rights reserved.
//

@import Cocoa;


@interface MWErrorColorTransformer : NSValueTransformer {

}

+ (Class)transformedValueClass;
+ (BOOL)allowsReverseTransformation;
- (id)transformedValue:(id)value;

@end
