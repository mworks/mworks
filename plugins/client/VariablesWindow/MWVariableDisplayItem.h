//
//  MWVariableDisplayItem.h
//  MWorksVariablesWindow
//
//  Created by Ben Kennedy on 8/28/07.
//  Copyright 2007 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface MWVariableDisplayItem : NSObject

@property(nonatomic, readonly, copy) NSString *displayName;
@property(nonatomic, readonly) NSInteger numberOfChildren;  // -1 for leaf nodes

- (id)initWithDisplayName:(NSString *)displayName;
- (void)setVariables:(NSArray *)variables;
- (MWVariableDisplayItem *)childAtIndex:(NSUInteger)index;

@end
