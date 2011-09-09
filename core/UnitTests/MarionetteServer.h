/*
 *  MarionetteServer.h
 *  MWorksCore
 *
 *  Created by bkennedy on 8/3/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#import <Cocoa/Cocoa.h>
#import "MWorksCore/Server.h"
using namespace mw;

@interface MarionetteServer : NSObject {
	shared_ptr<Server> core;
}

@end
