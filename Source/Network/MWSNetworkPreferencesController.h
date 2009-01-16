//
//  MWSNetworkPreferencesController.h
//  MonkeyWorksServer
//
//  Created by Ben Kennedy on 9/30/07.
//  Copyright 2007 MIT. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface MWSNetworkPreferencesController : NSObject {
	IBOutlet id delegate;
	
	IBOutlet NSPopUpButton *addresses;
	IBOutlet NSWindow *nwpWindow;
	IBOutlet NSTextField *currentAddress;
	IBOutlet NSTextField *defaultAddress;
	IBOutlet NSButton *applyButton;
}

- (void)awakeFromNib;
- (void)setDelegate:(id)_delegate;
- (id)delegate;

- (void)openAndInitWindow:(id)sender;

- (IBAction)openWindow:(id)sender;
- (IBAction)apply:(id)sender;
- (IBAction)addressChanged:(id)sender;

@end
