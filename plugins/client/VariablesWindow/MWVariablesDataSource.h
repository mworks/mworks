//
//  MWVariablesDataSource.h
//  MWorksVariablesWindow
//
//  Created by Ben Kennedy on 8/28/07.
//  Copyright 2007 MIT. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@class MWVariablesWindowController;  // Forward declaration


@interface MWVariablesDataSource : NSObject <NSOutlineViewDataSource, NSOutlineViewDelegate> {
	NSMutableArray *rootItems;
    NSMutableArray *expandedItems;
	IBOutlet NSTableColumn *nameCol;
	IBOutlet NSTableColumn *valueCol;
	
	IBOutlet MWVariablesWindowController *delegate;
}

- (MWVariablesWindowController *)delegate;
- (void)setDelegate:(MWVariablesWindowController *)new_delegate;

- (void)setRootGroups:(NSDictionary *)rootGroups forOutlineView:(NSOutlineView *)outlineView;

- (NSArray *)expandedItems;
- (void)setExpandedItems:(NSArray *)items forOutlineView:(NSOutlineView *)outlineView;

@end
