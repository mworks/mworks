//
//  MWVariableDisplayItem.h
//  MWorksVariablesWindow
//
//  Created by Ben Kennedy on 8/28/07.
//  Copyright 2007 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface MWVariableDisplayItem : NSObject
{
	NSString *displayName;
	NSMutableArray *children;
}

- (int)numberOfChildren;// Returns -1 for leaf nodes
- (MWVariableDisplayItem *)childAtIndex:(NSUInteger)n;// Invalid to call on leaf nodes
- (NSString *)displayName;

- (id) initWithGroupName:(NSString *)name
			andVariables:(NSArray *)vars;

- (id) initWithVarName:(NSString *)name;


@end