//
//  MWVariableDisplayItem.h
//  MWorksVariablesWindow
//
//  Created by Ben Kennedy on 8/28/07.
//  Copyright 2007 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface MWVariableDisplayItem : NSObject

@property(nonatomic, readonly, copy) NSString *name;
@property(nonatomic, readonly, copy) NSString *varDescription;
@property(nonatomic, copy) NSString *value;
@property(nonatomic, copy) NSArray<MWVariableDisplayItem *> *children;
@property(nonatomic, readonly) BOOL isGroup;
@property(nonatomic, readonly) NSInteger numberOfChildren;

- (instancetype)initWithName:(NSString *)name;
- (instancetype)initWithName:(NSString *)name varDescription:(NSString *)varDescription NS_DESIGNATED_INITIALIZER;

@end
