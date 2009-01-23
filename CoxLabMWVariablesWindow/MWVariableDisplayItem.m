//
//  MWVariableDisplayItem.m
//  MonkeyWorksVariablesWindow
//
//  Created by Ben Kennedy on 8/28/07.
//  Copyright 2007 __MyCompanyName__. All rights reserved.
//

#import "MWVariableDisplayItem.h"


@implementation MWVariableDisplayItem


- (id) initWithGroupName:(NSString *)name
			andVariables:(NSArray *)vars {
    if (self = [super init])
    {
		displayName = name;
		parent = nil;
		children = [[NSMutableArray alloc] initWithCapacity:[vars count]];

		NSString *newVarName;
		
		NSEnumerator *enumerator = [vars objectEnumerator];
		while(newVarName = [enumerator nextObject]) {
			MWVariableDisplayItem *newChild = [[MWVariableDisplayItem alloc] initWithVarName:newVarName
																				   andParent:self];
			[children addObject:newChild];
		}
    }
    return self;
}

- (id) initWithVarName:(NSString *)name
			 andParent:(id)_parent {
    if (self = [super init])
    {
		displayName = name;
		parent = _parent;
		children = nil;
    }
    return self;	
}

- (NSString *)displayName
{
    return displayName;
}


- (MWVariableDisplayItem *)childAtIndex:(int)n
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

- (MWVariableDisplayItem *)parent {	
	return parent;
}

@end

