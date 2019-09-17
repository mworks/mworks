//
//  ServerConnectedIconTransformer.h
//  New Client
//
//  Created by David Cox on 3/11/08.
//  Copyright 2008 Harvard University. All rights reserved.
//

@import Cocoa;


@interface MWServerConnectedIconTransformer : NSValueTransformer {

}

+ (Class)transformedValueClass;
+ (BOOL)allowsReverseTransformation;
- (id)transformedValue:(id)value;

@end
