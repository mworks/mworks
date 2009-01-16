//
//  MWSNetworkToolbarItem.h
//  MonkeyWorksServer
//
//  Created by labuser on 9/17/07.
//  Copyright 2007 MIT. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "MonkeyWorksCocoa/KBPopUpToolbarItem.h"


@interface MWSNetworkToolbarItem : KBPopUpToolbarItem {
	NSMenuItem *serverControl;
	NSMenuItem *acceptControl;
	NSMenu *submenu;
}

- (id)initWithItemIdentifier:(NSString *)itemIdentifier;
- (void)dealloc;
- (void)setDelegate:(id)_delegate;
- (void)updateDisplay:(id)arg;

@end
