//
//  MWVariablesDataSource.h
//  MonkeyWorksVariablesWindow
//
//  Created by Ben Kennedy on 8/28/07.
//  Copyright 2007 MIT. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface MWVariablesDataSource : NSObject {
	NSDictionary *rootGroups;
	NSMutableArray *rootItems;
	IBOutlet NSTableColumn *nameCol;
	IBOutlet NSTableColumn *valueCol;
	
	IBOutlet id delegate;
}

- (id)delegate;
- (void)setDelegate:(id)new_delegate;

- (void)addRootGroups:(NSDictionary *)groups;

@end
