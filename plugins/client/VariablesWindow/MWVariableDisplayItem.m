//
//  MWVariableDisplayItem.m
//  MWorksVariablesWindow
//
//  Created by Ben Kennedy on 8/28/07.
//  Copyright 2007 __MyCompanyName__. All rights reserved.
//

#import "MWVariableDisplayItem.h"


@implementation MWVariableDisplayItem


- (instancetype)initWithName:(NSString *)name {
    return [self initWithName:name varDescription:@""];
}


- (instancetype)initWithName:(NSString *)name varDescription:(NSString *)varDescription {
    self = [super init];
    if (self) {
        _name = [name copy];
        _varDescription = [varDescription copy];
        _value = @"";
    }
    return self;
}


- (BOOL)isGroup {
    return (self.children != nil);
}


- (NSInteger)numberOfChildren {
    if (self.children) {
        return self.children.count;
    }
    return 0;
}


@end
