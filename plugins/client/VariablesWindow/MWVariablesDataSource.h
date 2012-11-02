//
//  MWVariablesDataSource.h
//  MWorksVariablesWindow
//
//  Created by Ben Kennedy on 8/28/07.
//  Copyright 2007 MIT. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@class MWVariablesWindowController;  // Forward declaration


@interface MWVariablesDataSource : NSObject <NSOutlineViewDataSource> {
	NSMutableArray *rootItems;
	IBOutlet NSTableColumn *nameCol;
	IBOutlet NSTableColumn *valueCol;
	
	IBOutlet MWVariablesWindowController *delegate;
}

- (MWVariablesWindowController *)delegate;
- (void)setDelegate:(MWVariablesWindowController *)new_delegate;

- (void)setRootGroups:(NSDictionary *)rootGroups;

@end
