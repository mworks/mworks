//
//  MarionetteServer.m
//  MWorksCore
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
		core = shared_ptr<Server>(new Server());
		core->setListenPort(DEFAULT_PORT);
		core->setHostname(DEFAULT_HOST_IP);
		core->startServer();
	}
	return self;
}

- (void)dealloc {
    core.reset();
}



@end
