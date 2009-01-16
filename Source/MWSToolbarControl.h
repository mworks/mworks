//
//  MWSToolbarControl.h
//  MonkeyWorksServer
//
//  Created by labuser on 9/17/07.
//  Copyright 2007 MIT. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface MWSToolbarControl : NSObject {
	IBOutlet NSWindow *window;
	IBOutlet id delegate;
	NSToolbar *toolbar;
}

- (id)delegate;
- (void)setDelegate:(id)delegate;
- (void)awakeFromNib;
- (void)updateDisplay;

@end

@interface MWSToolbarControl(DelegateMethods)
- (NSToolbarItem *)toolbar:(NSToolbar *)toolbar
	 itemForItemIdentifier:(NSString *)itemIdentifier
	willBeInsertedIntoToolbar:(bool)flag;
- (NSArray *)toolbarAllowedItemIdentifiers:(NSToolbar *)toolbar;
- (NSArray *)toolbarDefaultItemIdentifiers:(NSToolbar *)toolbar;
- (NSArray *)toolbarSelectableItemIdentifiers:(NSToolbar *)toolbar;
- (void)stopServer:(id)sender;
- (void)startServer:(id)sender;
- (void)startAccepting:(id)sender;
- (void)stopAccepting:(id)sender;
- (void)openNetworkPreferences:(id)sender;
- (NSNumber *)serverStarted:(id)arg;
- (NSNumber *)serverAccepting:(id)arg;
@end