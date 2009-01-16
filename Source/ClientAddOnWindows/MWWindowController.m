/**
 * MWWindowController.m
 *
 * History:
 * Paul Jankunas on 8/18/05 - Created.
 *
 * Copyright 2005 MIT. All rights reserved.
 */

#import "MWWindowController.h"

NSString * PreferenceGroupWindowPositions = @"WindowPositions";

@implementation MWWindowController

- (id)delegate {
	return delegate;
}

- (void)setDelegate:(id)_delegate {
	delegate = _delegate;
}

/**********************************************************************
 *                  NSWindowController Added Methods
 *********************************************************************/
- (NSString *)monkeyWorksFrameAutosaveName {
    return nil;
}

- (void)popWindow {
    NSWindow * win = [self window];
    if([win isVisible]) {
        // the window is visible hide it
        [win orderOut:win];
    } else {
        [win orderFront:win];
        [win makeKeyWindow];
    }
}

/**********************************************************************
 *                  NSWindowController Overridden Methods
 *********************************************************************/
- (BOOL)windowShouldClose:(id)sender {
	return YES;
}

- (BOOL)shouldCascadeWindows {
    return NO;
}

- (void)close {
    [[self window] performClose:nil];
}

@end
