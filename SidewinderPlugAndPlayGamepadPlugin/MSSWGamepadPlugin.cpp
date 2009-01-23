/*
 *  MSSWGamepadPlugin.cpp
 *  MSSWGamepadPlugin
 *
 *  Created by bkennedy on 8/14/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#include "MSSWGamepadPlugin.h"
#include "MSSWGamepadFactory.h"

Plugin *getPlugin(){
    return new mMSSWGamepadPlugin();
}


void mMSSWGamepadPlugin::registerComponents(shared_ptr<mw::mwComponentRegistry> registry) {
	registry->registerFactory(std::string("iodevice/sidewinder_gamepad"),
							  (ComponentFactory *)(new mMSSWGamepadFactory()));
}
