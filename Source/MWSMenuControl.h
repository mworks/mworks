//
//  MWSMenuControl.h
//  MonkeyWorksServer
//
//  Created by labuser on 9/17/07.
//  Copyright 2007 MIT. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "MWSToolbarControl.h"

@interface MWSMenuControl : NSObject {
	IBOutlet id delegate;

	IBOutlet NSMenuItem *runStopControl;
	IBOutlet NSMenuItem *menuServerControl;
	IBOutlet NSMenuItem *menuAcceptControl;
}

- (id)delegate;
- (void)setDelegate:(id)new_delegate;
- (void)updateDisplay;


@end
