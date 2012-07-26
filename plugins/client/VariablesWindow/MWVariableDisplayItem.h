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

@property(nonatomic, readonly) NSString *displayName;
@property(nonatomic, readonly) int numberOfChildren;  // -1 for leaf nodes

- (id)initWithName:(NSString *)name;
- (void)setVariables:(NSArray *)vars;
- (MWVariableDisplayItem *)childAtIndex:(NSUInteger)n;  // Invalid to call on leaf nodes

@end
