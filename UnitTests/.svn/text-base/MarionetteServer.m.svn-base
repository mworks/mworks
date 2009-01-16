//
//  MarionetteServer.m
//  MonkeyWorksCore
//
//  Created by bkennedy on 8/3/08.
//  Copyright 2008 MIT. All rights reserved.
//

#import "MarionetteServer.h"
#import "MarionetteGlobals.h"

@implementation MarionetteServer

- (id) init {
	self = [super init];
	if (self != nil) {		
		core = new Server();
		
		core->setListenLowPort(DEFAULT_LOW_PORT);
		core->setListenHighPort(DEFAULT_HIGH_PORT);
		
		core->setHostname(DEFAULT_HOST_IP);
		core->startServer();
		core->startAccepting();	
	}
	return self;
}

- (void)dealloc {
	delete core;
	[super dealloc];
}



@end
