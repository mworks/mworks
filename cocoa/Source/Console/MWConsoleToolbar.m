//
//  MWConsoleToolbar.m
//  MWorksCocoa
//
//  Created by Ben Kennedy on 10/5/07.
//  Copyright 2007 __MyCompanyName__. All rights reserved.
//

#import "MWConsoleToolbar.h"
#import "MWConsoleController.h"
#import <Appkit/Appkit.h>

#define SAVE_BUTTON @"MWorksCocoa console save button"
#define CLEAR_BUTTON @"MWorksCocoa console clear button"

@implementation MWConsoleToolbar
- (id)delegate {
	return delegate;
}

- (void)setDelegate:(id)_delegate {
	delegate = _delegate;
}

- (void)awakeFromNib {
	toolbar = [[NSToolbar alloc] initWithIdentifier:@"MWConsoleWindowToolbar"];
	[toolbar setDisplayMode:NSToolbarDisplayModeIconAndLabel];
	[toolbar setShowsBaselineSeparator:NO];
	[toolbar setSizeMode:NSToolbarSizeModeRegular];
	[toolbar setAllowsUserCustomization:NO];
	[toolbar setAutosavesConfiguration:NO];
	[toolbar setDelegate:self];	
	[window setToolbar:[toolbar autorelease]];
  
}

////////////////////////////////////////
// Delegate methods
////////////////////////////////////////
- (NSToolbarItem *)toolbar:(NSToolbar *)toolbar
	 itemForItemIdentifier:(NSString *)itemIdentifier
	willBeInsertedIntoToolbar:(BOOL)flag {
	
	if([itemIdentifier isEqualToString:SAVE_BUTTON]) {
		NSString *resourcePath = [[NSBundle bundleWithPath:@"/Library/Frameworks/MWorksCocoa.framework"] resourcePath];
		NSToolbarItem *item = [[NSToolbarItem alloc] initWithItemIdentifier:itemIdentifier];
		
		[item setLabel:@"Save"];
		[item setPaletteLabel:[item label]];
		//[item setImage:[[[NSImage alloc] initByReferencingFile:[resourcePath stringByAppendingPathComponent:@"Disk.png"]] autorelease]]
    [item setImage:[[NSImage alloc] initByReferencingFile:[resourcePath stringByAppendingPathComponent:@"Disk.png"]]];
		[item setTarget:delegate];
		[item setAction:@selector(saveLog:)];
		
		//return [item autorelease];
    return item;
	}  else if([itemIdentifier isEqualToString:CLEAR_BUTTON]) {
		NSString *resourcePath = [[NSBundle bundleWithPath:@"/Library/Frameworks/MWorksCocoa.framework"] resourcePath];
		NSToolbarItem *item = [[NSToolbarItem alloc] initWithItemIdentifier:itemIdentifier];
		
		[item setLabel:@"Clear"];
		[item setPaletteLabel:[item label]];
		//[item setImage:[[[NSImage alloc] initByReferencingFile:[resourcePath stringByAppendingPathComponent:@"Broom.tif"]] autorelease]];
    [item setImage:[[NSImage alloc] initByReferencingFile:[resourcePath stringByAppendingPathComponent:@"Broom.tif"]] ];
		[item setTarget:delegate];
		[item setAction:@selector(clearLog:)];
		
		//return [item autorelease];
    return item;
	} 
	return nil;
}

- (NSArray *)toolbarAllowedItemIdentifiers:(NSToolbar *)toolbar {
	return [NSArray arrayWithObjects:NSToolbarFlexibleSpaceItemIdentifier,
		SAVE_BUTTON,
		CLEAR_BUTTON,
		nil];
}

- (NSArray *)toolbarDefaultItemIdentifiers:(NSToolbar *)toolbar {
	return [NSArray arrayWithObjects:
		SAVE_BUTTON,
		CLEAR_BUTTON,
		nil];
}


@end
