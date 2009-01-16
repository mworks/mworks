//
//  MWSToolbarControl.m
//  MonkeyWorksServer
//
//  Created by labuser on 9/17/07.
//  Copyright 2007 MIT. All rights reserved.
//

#import "MWSToolbarControl.h"
#import "MWSServer.h"
#import "MWSNetworkToolbarItem.h"
#import "MWSConstants.h"

@implementation MWSToolbarControl

- (id)delegate {
	return delegate;
}

- (void)setDelegate:(id)_delegate {
	if(![_delegate respondsToSelector:@selector(openNetworkPreferences:)] ||
	   ![_delegate respondsToSelector:@selector(serverStarted)] ||
	   ![_delegate respondsToSelector:@selector(serverAccepting)] ||
	   ![_delegate respondsToSelector:@selector(startServer)] ||
	   ![_delegate respondsToSelector:@selector(startAccepting)] ||
	   ![_delegate respondsToSelector:@selector(stopServer)] ||
	   ![_delegate respondsToSelector:@selector(stopAccepting)]) {
		[NSException raise:NSInternalInconsistencyException
					format:@"Delegate doesn't respond to required methods for MWSToolbarControl"];		
	}	
	
	delegate = _delegate;
}

- (void)awakeFromNib {
	toolbar = [[NSToolbar alloc] initWithIdentifier:@"MWSMainWindowToolbar"];
	[toolbar setDisplayMode:NSToolbarDisplayModeIconAndLabel];
	[toolbar setShowsBaselineSeparator:NO];
	[toolbar setSizeMode:NSToolbarSizeModeRegular];
	[toolbar setAllowsUserCustomization:NO];
	[toolbar setAutosavesConfiguration:NO];
	[toolbar setDelegate:self];	
	[window setToolbar:toolbar];
}

- (void)updateDisplay {
	NSEnumerator *enumerator = [[toolbar items] objectEnumerator];
	MWToolbarItem *item;
	
	while(item = [enumerator nextObject]) {
		if([item respondsToSelector:@selector(updateDisplay:)]) {
			[item performSelectorOnMainThread:@selector(updateDisplay:)
								   withObject:nil
								waitUntilDone:YES];
		}
	}
}

////////////////////////////////////////
// Delegate methods
////////////////////////////////////////
- (NSToolbarItem *)toolbar:(NSToolbar *)toolbar
	 itemForItemIdentifier:(NSString *)itemIdentifier
	willBeInsertedIntoToolbar:(bool)flag {
	
	if([itemIdentifier isEqualToString:NETWORK_BUTTON]) {
		MWSNetworkToolbarItem *item = [[MWSNetworkToolbarItem alloc] initWithItemIdentifier:itemIdentifier];
		[item setDelegate:self];
		return [item autorelease];
	} else if([itemIdentifier isEqualToString:CONSOLE_BUTTON]) {
		NSString *resourcePath = [[NSBundle bundleWithPath:@"/Library/Frameworks/MonkeyWorksCocoa.framework"] resourcePath];
		NSToolbarItem *item = [[NSToolbarItem alloc] initWithItemIdentifier:itemIdentifier];
		
		[item setLabel:@"Console"];
		[item setPaletteLabel:[item label]];
		[item setImage:[[[NSImage alloc] initByReferencingFile:[resourcePath stringByAppendingPathComponent:@"Console.tif"]] autorelease]];
		[item setTarget:delegate];
		[item setAction:@selector(toggleConsole:)];
		
		return [item autorelease];
	}
	return nil;
}

- (NSArray *)toolbarAllowedItemIdentifiers:(NSToolbar *)toolbar {
	return [NSArray arrayWithObjects:NSToolbarFlexibleSpaceItemIdentifier,
		NETWORK_BUTTON, 
		CONSOLE_BUTTON, 
		nil];
}

- (NSArray *)toolbarDefaultItemIdentifiers:(NSToolbar *)toolbar {
	return [NSArray arrayWithObjects:
		NETWORK_BUTTON, 
		NSToolbarFlexibleSpaceItemIdentifier,
		CONSOLE_BUTTON, 
		nil];
}

- (void)stopServer:(id)sender {
	if(delegate != nil) {
		[delegate stopServer];
	}
}

- (void)startServer:(id)sender {
	if(delegate != nil) {
		[delegate startServer];	
	}
}

- (void)startAccepting:(id)sender {
	if(delegate != nil) {
		[delegate startAccepting];		
	}
}

- (void)stopAccepting:(id)sender {
	if(delegate != nil) {
		[delegate stopAccepting];			
	}
}

- (void)openNetworkPreferences:(id)sender {
	if(delegate != nil) {
		[delegate openNetworkPreferences:sender];
	}	
}

- (NSNumber *)serverStarted:(id)arg {
	if(delegate != nil) {
		return [delegate serverStarted];
	} else {
		return nil;
	}
}

- (NSNumber *)serverAccepting:(id)arg {
	if(delegate != nil) {
		return [delegate serverAccepting];
	} else {
		return nil;
	}
}

@end
