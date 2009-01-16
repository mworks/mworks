//
//  MWSNetworkToolbarItem.m
//  MonkeyWorksServer
//
//  Created by labuser on 9/17/07.
//  Copyright 2007 MIT. All rights reserved.
//

#import "MWSNetworkToolbarItem.h"
#import "MWSToolbarControl.h"
#import "MWSServer.h"
#import "MWSConstants.h"

@interface MWSNetworkToolbarItem (PrivateMethods)
- (void)startServer:(id)sender;
- (void)startAccepting:(id)sender;
- (void)stopAccepting:(id)sender;
- (void)stopServer:(id)sender;
- (void)openNetworkPreferences:(id)sender;
@end

@implementation MWSNetworkToolbarItem

- (id)initWithItemIdentifier:(NSString *)itemIdentifier {
	self = [super initWithItemIdentifier:itemIdentifier];
	if (self != nil) {
		
		[self setLabel:@"Network"];
		[self setPaletteLabel:[self label]];
		[self setToolTip:@"Access the network controls"];
		[self setTarget:self];
		NSString *resourcePath = [[NSBundle bundleWithPath:@"/Library/Frameworks/MonkeyWorksCocoa.framework"] resourcePath];
		[self setImage:[[[NSImage alloc] initByReferencingFile:[resourcePath stringByAppendingPathComponent:@"greenCheck.tif"]] autorelease]];
		submenu=[[[NSMenu alloc] init] autorelease];
		serverControl=[[[NSMenuItem alloc] initWithTitle:STOP_SERVER
												 action:@selector(stopServer)
										  keyEquivalent: @""] autorelease];
		
		acceptControl=[[[NSMenuItem alloc] initWithTitle:STOP_ACCEPTING
												 action:@selector(stopAccepting)
										  keyEquivalent: @""] autorelease];
		
		NSMenuItem *networkPreferencesControl = [[[NSMenuItem alloc] initWithTitle:@"Network Preferences"
																			action:@selector(openNetworkPreferences:)
																	keyEquivalent: @""] autorelease];
		
		[networkPreferencesControl setTarget:self];
		[networkPreferencesControl setEnabled:YES];
		
		[submenu addItem:serverControl];
		[submenu addItem:acceptControl];
		[submenu addItem:[NSMenuItem separatorItem]];
		[submenu addItem:networkPreferencesControl];
		[acceptControl setTarget:self];
		[acceptControl setEnabled:YES];
		[serverControl setTarget:self];
		[serverControl setEnabled:YES];
		[self setMenu:submenu];
	}
	return self;
}

- (void)dealloc {
	[serverControl release];
	[acceptControl release];
	[submenu release];
	[super dealloc];
}

- (void)setDelegate:(id)_delegate {
	if(![_delegate respondsToSelector:@selector(openNetworkPreferences:)] ||
	   ![_delegate respondsToSelector:@selector(serverStarted:)] ||
	   ![_delegate respondsToSelector:@selector(serverAccepting:)] ||
	   ![_delegate respondsToSelector:@selector(startServer:)] ||
	   ![_delegate respondsToSelector:@selector(startAccepting:)] ||
	   ![_delegate respondsToSelector:@selector(stopServer:)] ||
	   ![_delegate respondsToSelector:@selector(stopAccepting:)]) {
		[NSException raise:NSInternalInconsistencyException
					format:@"Delegate doesn't respond to required methods for MWSNetworkToolbarItem"];		
	}
	
	delegate = _delegate;
}

- (void)updateDisplay:(id)arg {
	NSString *resourcePath = [[NSBundle bundleWithPath:@"/Library/Frameworks/MonkeyWorksCocoa.framework"] resourcePath];
	
	[acceptControl setTarget:self];
	[acceptControl setEnabled:YES];
	[serverControl setTarget:self];
	[serverControl setEnabled:YES];
	if(delegate != nil) {
		if([[delegate serverAccepting:nil] boolValue]) {
			[self setImage:[[[NSImage alloc] initByReferencingFile:[resourcePath stringByAppendingPathComponent:@"greenCheck.tif"]] autorelease]];
			[acceptControl setTitle:STOP_ACCEPTING];
			[acceptControl setAction:@selector(stopAccepting:)];
		} else {
			[self setImage:[[[NSImage alloc] initByReferencingFile:[resourcePath stringByAppendingPathComponent:@"yellowCheck.tif"]] autorelease]];
			[acceptControl setTitle:START_ACCEPTING];
			[acceptControl setAction:@selector(startAccepting:)];		
		}
		
		if([[delegate serverStarted:nil] boolValue]) {
			[self setImage:[[[NSImage alloc] initByReferencingFile:[resourcePath stringByAppendingPathComponent:@"greenCheck.tif"]] autorelease]];
			[serverControl setTitle:STOP_SERVER];
			[serverControl setAction:@selector(stopServer:)];
		} else {
			[self setImage:[[[NSImage alloc] initByReferencingFile:[resourcePath stringByAppendingPathComponent:@"redX.tif"]] autorelease]];
			[serverControl setTitle:START_SERVER];
			[serverControl setAction:@selector(startServer:)];	
		}
	}
}

////////////////////////////////////////////////
// private methods
//////////////////////////////////////////////
- (void)startServer:(id)sender {
	if(delegate != nil) {
		[delegate startServer:sender];
	}
}

- (void)startAccepting:(id)sender {
	if(delegate != nil) {
		[delegate startAccepting:sender];
	}
}

- (void)stopAccepting:(id)sender {
	if(delegate != nil) {
		[delegate stopAccepting:sender];
	}
}

- (void)stopServer:(id)sender {
	if(delegate != nil) {
		[delegate stopServer:sender];
	}
}

- (void)openNetworkPreferences:(id)sender {
	if(delegate != nil) {
		[delegate openNetworkPreferences:sender];
	}
}



@end
