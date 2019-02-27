//
//  MWExperimentPlayStopIconTransformer.m
//  New Client
//
//  Created by David Cox on 3/12/08.
//  Copyright 2008 Harvard University. All rights reserved.
//

#import "MWExperimentPlayStopIconTransformer.h"


@implementation MWExperimentPlayStopIconTransformer


+ (Class)transformedValueClass
{
    return [NSImage class];
}


- (id)transformedValue:(id)value
{
    NSString *imageName;
    
    if ([value respondsToSelector:@selector(boolValue)] && [value boolValue]) {
        imageName = [[NSBundle mainBundle] pathForResource:@"stop" ofType:@"png"];
    } else {
        imageName = [[NSBundle mainBundle] pathForResource:@"play" ofType:@"png"];
    }
    
    return [[NSImage alloc] initWithContentsOfFile:imageName];
}


@end
