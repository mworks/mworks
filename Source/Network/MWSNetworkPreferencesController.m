//
//  MWNetworkPreferencesController.m
//  MonkeyWorksServer
//
//  Created by Ben Kennedy on 9/30/07.
//  Copyright 2007 MIT. All rights reserved.
//

#import "MWSNetworkPreferencesController.h"
#import "MWSServer.h"
#import <sys/types.h>
#import <sys/socket.h>
#import <ifaddrs.h>

@interface MWSNetworkPreferencesController(PrivateMethods)
- (void)alertDidEnd:(NSAlert *)alert 
		 returnCode:(int)returnCode 
		contextInfo:(void *)contextInfo;
@end

@implementation MWSNetworkPreferencesController

- (void)awakeFromNib {

	// TODO: this is crazy slow on some machines/networks
	#define ESCHEW_NSHOST	1
	#if ESCHEW_NSHOST
		// TODO: double check this
		struct ifaddrs *addrs;
		int i = getifaddrs(&addrs);
		NSMutableArray *netAddresses = [[NSMutableArray alloc] init];
		while(addrs != NULL){
			[netAddresses insertObject:[NSString stringWithCString:addrs->ifa_name] atIndex:0];
			addrs = addrs->ifa_next;
		}
	#else	
		NSArray *netAddresses = [[NSHost currentHost] addresses];
	#endif
	
	NSString *regex  = @"[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}";
	NSString *predicateFormat = [[@"SELF MATCHES \"" stringByAppendingString:regex] stringByAppendingString:@"\""];
	
	NSPredicate *addressPredicate =
		[NSPredicate predicateWithFormat:predicateFormat];
	
	NSArray *filteredArray = [netAddresses filteredArrayUsingPredicate:addressPredicate];

	[addresses removeAllItems];
	
	if([filteredArray count] > 0) {
		[addresses addItemsWithTitles:filteredArray];
		[addresses setEnabled:YES];
	} else {
		[addresses addItemWithTitle:@"Network interface not detected"];
		[addresses setEnabled:NO];
	}
}

- (void)setDelegate:(id)_delegate {
	if (![_delegate respondsToSelector:@selector(setListeningAddress:)] ||
		![_delegate respondsToSelector:@selector(defaultNetworkAddress:)] ||
		![_delegate respondsToSelector:@selector(currentNetworkAddress:)]) {
        [NSException raise:NSInternalInconsistencyException
					format:@"Delegate doesn't respond to required methods for MWSNetworkPreferencesController"];
    }
	
	
	delegate = _delegate;
}

- (id)delegate {
	return delegate;
}


- (void)openAndInitWindow:(id)sender {
	[applyButton setEnabled:![[addresses titleOfSelectedItem] isEqualToString:[delegate currentNetworkAddress:self]]];
	if(delegate != nil) {
		[currentAddress setStringValue:[delegate currentNetworkAddress:self]];
		[defaultAddress setStringValue:[delegate defaultNetworkAddress:self]];
	}
	[nwpWindow makeKeyAndOrderFront:sender];
}

- (IBAction)apply:(id)sender {
	[delegate setListeningAddress:[defaultAddress stringValue]];
//	[defaultAddress setStringValue:[delegate defaultNetworkAddress:self]];
	[applyButton setEnabled:![[addresses titleOfSelectedItem] isEqualToString:[delegate defaultNetworkAddress:self]]];
	
	NSAlert *alert = [[[NSAlert alloc] init] autorelease];
	[alert addButtonWithTitle:@"Restart"];
	[alert addButtonWithTitle:@"Don't Restart"];
	[alert setMessageText:@"Restart MonkeyWorksServer?"];
	[alert setInformativeText:@"MonkeyWorksServer must be restarted for changes to take effect."];
	[alert setAlertStyle:NSWarningAlertStyle];
	
	[alert beginSheetModalForWindow:nwpWindow modalDelegate:self didEndSelector:@selector(alertDidEnd:returnCode:contextInfo:) contextInfo:nil];	
}

- (IBAction)openWindow:(id)sender {
	[self openAndInitWindow:sender];
}

- (IBAction)addressChanged:(id)sender {
	if(delegate != nil) {
		[applyButton setEnabled:![[addresses titleOfSelectedItem] isEqualToString:[delegate defaultNetworkAddress:self]]];
	}
}

- (void)alertDidEnd:(NSAlert *)alert
		 returnCode:(int)returnCode 
		contextInfo:(void *)contextInfo {
    if (returnCode == NSAlertFirstButtonReturn) {
		
		NSURL *url = [NSURL fileURLWithPath:[[NSBundle mainBundle] bundlePath]];
		
		LSLaunchURLSpec launchSpec;
		launchSpec.appURL = (CFURLRef)url;
		launchSpec.itemURLs = NULL;
		launchSpec.passThruParams = NULL;
		launchSpec.launchFlags = kLSLaunchDefaults | kLSLaunchNewInstance;
		launchSpec.asyncRefCon = NULL;
		
		OSErr err = LSOpenFromURLSpec(&launchSpec, NULL);
		if (err == noErr) {
			[NSApp terminate:nil];
		} else {
			// Handle relaunch failure
		}
//		exit(0);
    } 
}

@end
