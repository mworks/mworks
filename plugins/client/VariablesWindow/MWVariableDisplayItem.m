//
//  MWVariableDisplayItem.m
//  MWorksVariablesWindow
//
//  Created by Ben Kennedy on 8/28/07.
//  Copyright 2007 __MyCompanyName__. All rights reserved.
//

#import "MWVariableDisplayItem.h"


@implementation MWVariableDisplayItem


@synthesize displayName;


- (id)initWithName:(NSString *)name
{
    if ((self = [super init])) {
        displayName = [name copy];
    }
    return self;
}


- (void)setVariables:(NSArray *)vars
{
    [children release];
    children = [[NSMutableArray alloc] initWithCapacity:[vars count]];
    
    for (NSString *newVarName in vars) {
        MWVariableDisplayItem *newChild = [[MWVariableDisplayItem alloc] initWithName:newVarName];
        [children addObject:newChild];
        [newChild release];
    }
}


- (MWVariableDisplayItem *)childAtIndex:(NSUInteger)n
{
	if(children) {
		return [children objectAtIndex:n];
	} else {
		return nil;
	}
}


- (int)numberOfChildren
{
	if (children == nil) {
		return -1;
	} else {
		return [children count];
	}
}


- (void)dealloc
{
    [children release];
    [displayName release];
    [super dealloc];
}


@end


























