//
//  MWConsoleToolbar.h
//  MonkeyWorksCocoa
//
//  Created by Ben Kennedy on 10/5/07.
//  Copyright 2007 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface MWConsoleToolbar : NSObject {
	IBOutlet NSWindow *window;
	IBOutlet id delegate;
	NSToolbar *toolbar;
}

- (id)delegate;
- (void)setDelegate:(id)delegate;
- (void)awakeFromNib;

@end

@interface MWConsoleToolbar(DelegateMethods)
- (NSToolbarItem *)toolbar:(NSToolbar *)toolbar
	 itemForItemIdentifier:(NSString *)itemIdentifier
	willBeInsertedIntoToolbar:(bool)flag;
- (NSArray *)toolbarAllowedItemIdentifiers:(NSToolbar *)toolbar;
- (NSArray *)toolbarDefaultItemIdentifiers:(NSToolbar *)toolbar;
- (NSArray *)toolbarSelectableItemIdentifiers:(NSToolbar *)toolbar;
@end
