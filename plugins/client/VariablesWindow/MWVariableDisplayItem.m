//
//  MWVariableDisplayItem.m
//  MWorksVariablesWindow
//
//  Created by Ben Kennedy on 8/28/07.
//  Copyright 2007 __MyCompanyName__. All rights reserved.
//

#import "MWVariableDisplayItem.h"


@implementation MWVariableDisplayItem {
    NSMutableArray *children;
}


- (id)initWithDisplayName:(NSString *)displayName {
    if ((self = [super init])) {
        _displayName = [displayName copy];
    }
    return self;
}


- (void)setVariables:(NSArray *)variables {
    children = [[NSMutableArray alloc] initWithCapacity:[variables count]];
    
    for (NSString *newVarName in variables) {
        MWVariableDisplayItem *newChild = [[MWVariableDisplayItem alloc] initWithDisplayName:newVarName];
        [children addObject:newChild];
    }
}


- (MWVariableDisplayItem *)childAtIndex:(NSUInteger)index {
    if (children) {
        return children[index];
    }
    return nil;
}


- (NSInteger)numberOfChildren {
    if (children) {
        return [children count];
    }
    return -1;
}


@end


























