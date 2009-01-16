//
//  NSMenuExtensions.h
//  New Client
//
//  Created by David Cox on 3/16/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface NSMenu (PopUpRegularMenuAdditions)
+ (void)popUpMenu:(NSMenu *)menu forView:(NSView *)view pullsDown:(BOOL)pullsDown;
@end

