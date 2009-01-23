//
//  MWVariableDisplayItem.h
//  MonkeyWorksVariablesWindow
//
//  Created by Ben Kennedy on 8/28/07.
//  Copyright 2007 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface MWVariableDisplayItem : NSObject
{
	NSString *displayName;
	NSMutableArray *children;
    MWVariableDisplayItem *parent;
}

- (int)numberOfChildren;// Returns -1 for leaf nodes
- (MWVariableDisplayItem *)childAtIndex:(int)n;// Invalid to call on leaf nodes
- (NSString *)displayName;
- (MWVariableDisplayItem *)parent;

- (id) initWithGroupName:(NSString *)name
			andVariables:(NSArray *)vars;

- (id) initWithVarName:(NSString *)name
			andParent:(id)parent;


@end