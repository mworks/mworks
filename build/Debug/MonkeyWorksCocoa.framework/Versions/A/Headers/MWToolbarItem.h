//
//  MWCToolbarItem.h
//  MonkeyWorksClient
//
//  Created by bkennedy on 9/19/07. <-- National "Talk like a pirate day"
//  Copyright 2007 MIT. All rights reserved.  Arrrrr!
//

#import <Cocoa/Cocoa.h>


@interface MWToolbarItem : NSToolbarItem {
	id delegate;
}

- (id)initWithItemIdentifier:(NSString *)itemIdentifier;
- (id)delegate;
- (void)setDelegate:(id)_delegate;
- (void)updateDisplay:(id)arg;

@end
