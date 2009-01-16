//
//  NSMenuExtensions.m
//  New Client
//
//  Created by David Cox on 3/16/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import "NSMenuExtensions.h"


@implementation NSMenu (PopUpRegularMenuAdditions)
+ (void)popUpMenu:(NSMenu *)menu forView:(NSView *)view pullsDown:(BOOL)pullsDown {
    NSRect frame = [view frame];
    frame.origin.x = 0.0;
    frame.origin.y = 0.0;

    if (pullsDown) [menu insertItemWithTitle:@"" action:NULL keyEquivalent:@"" atIndex:0];

    NSPopUpButtonCell *popUpButtonCell = [[NSPopUpButtonCell alloc] initTextCell:@"" pullsDown:pullsDown];
    [popUpButtonCell setMenu:menu];
    if (!pullsDown) [popUpButtonCell selectItem:nil];
    [popUpButtonCell performClickWithFrame:frame inView:view];
}
@end
