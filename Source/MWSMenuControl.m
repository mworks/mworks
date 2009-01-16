//
//  MWSMenuControl.m
//  MonkeyWorksServer
//
//  Created by labuser on 9/17/07.
//  Copyright 2007 MIT. All rights reserved.
//

#import "MWSMenuControl.h"
#import "MWSServer.h"
#import "MWSConstants.h"

@implementation MWSMenuControl

- (id)delegate {
    return delegate;
}

- (void)setDelegate:(id)_delegate {
	if (![_delegate respondsToSelector:@selector(serverAccepting)] ||
		![_delegate respondsToSelector:@selector(serverStarted)] ||
		![_delegate respondsToSelector:@selector(stopAccepting)] ||
		![_delegate respondsToSelector:@selector(startAccepting)]) {
        [NSException raise:NSInternalInconsistencyException
					format:@"Delegate doesn't respond to required methods for MWSMenuControl"];
    }

    delegate = _delegate;
}


- (void)updateDisplay {
	if(delegate != nil) {
		[menuAcceptControl setTarget:delegate];
		[menuServerControl setTarget:delegate];
		[runStopControl setTarget:delegate];
		
		if([[delegate serverAccepting] boolValue]) {
			
			[menuAcceptControl setTitle:STOP_ACCEPTING];
			[menuAcceptControl setAction:@selector(stopAccepting)];
		} else {
			[menuAcceptControl setTitle:START_ACCEPTING];
			[menuAcceptControl setAction:@selector(startAccepting)];
		}
		
		if ([[delegate serverStarted] boolValue]) {
			[menuServerControl setTitle:STOP_SERVER];		
			[menuServerControl setAction:@selector(stopServer)];
		} else {
			[menuServerControl setTitle:START_SERVER];			
			[menuServerControl setAction:@selector(startServer)];
		}
	}
}

@end
