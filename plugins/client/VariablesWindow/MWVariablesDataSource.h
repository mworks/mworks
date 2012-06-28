//
//  MWVariablesDataSource.h
//  MWorksVariablesWindow
//
//  Created by Ben Kennedy on 8/28/07.
//  Copyright 2007 MIT. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface MWVariablesDataSource : NSObject <NSOutlineViewDataSource> {
	NSMutableArray *rootItems;
	IBOutlet NSTableColumn *nameCol;
	IBOutlet NSTableColumn *valueCol;
	
	IBOutlet id delegate;
}

- (id)delegate;
- (void)setDelegate:(id)new_delegate;

- (void)setRootGroups:(NSDictionary *)rootGroups;

@end
