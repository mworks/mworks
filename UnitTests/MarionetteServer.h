/*
 *  MarionetteServer.h
 *  MonkeyWorksCore
 *
 *  Created by bkennedy on 8/3/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#import <Cocoa/Cocoa.h>
#import "MonkeyWorksCore/Server.h"
using namespace mw;

@interface MarionetteServer : NSObject {
	Server *core;
}

@end
