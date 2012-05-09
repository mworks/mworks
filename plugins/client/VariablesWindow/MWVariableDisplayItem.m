//
//  MWVariableDisplayItem.m
//  MWorksVariablesWindow
//
//  Created by Ben Kennedy on 8/28/07.
//  Copyright 2007 __MyCompanyName__. All rights reserved.
//

#import "MWVariableDisplayItem.h"


@implementation MWVariableDisplayItem


- (id) initWithGroupName:(NSString *)name
			andVariables:(NSArray *)vars
{
    if (self = [super init])
    {
		displayName = name;
		children = [[NSMutableArray alloc] initWithCapacity:[vars count]];

		NSString *newVarName;
		
		NSEnumerator *enumerator = [vars objectEnumerator];
		while(newVarName = [enumerator nextObject]) {
			MWVariableDisplayItem *newChild = [[MWVariableDisplayItem alloc] initWithVarName:newVarName];
			[children addObject:newChild];
            [newChild release];
		}
    }
    return self;
}

- (id) initWithVarName:(NSString *)name
{
    if (self = [super init])
    {
		displayName = name;
		children = nil;
    }
    return self;	
}

- (NSString *)displayName
{
    return displayName;
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
    if (children) [children release];
    [displayName release];
    [super dealloc];
}

@end

