//
//  MWExperimentPauseResumeIconTransformer.m
//  MWClient
//
//  Created by Christopher Stawarz on 5/30/12.
//  Copyright (c) 2012 MIT. All rights reserved.
//

#import "MWExperimentPauseResumeIconTransformer.h"


@implementation MWExperimentPauseResumeIconTransformer


+ (Class)transformedValueClass
{
    return [NSImage class];
}


- (id)transformedValue:(id)value
{
    NSString *imageName;
    
    if ([value respondsToSelector:@selector(boolValue)] && [value boolValue]) {
        imageName = [[NSBundle mainBundle] pathForResource:@"play" ofType:@"png"];
    } else {
        imageName = [[NSBundle mainBundle] pathForResource:@"pause" ofType:@"png"];
    }
    
    return [[NSImage alloc] initWithContentsOfFile:imageName];
}


@end
