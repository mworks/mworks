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
    if ((self = [super init])) {
        _name = [name copy];
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


























