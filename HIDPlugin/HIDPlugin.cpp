/*
 *  HIDPlugin.cpp
 *  HIDPlugin
 *
 *  Created by bkennedy on 8/14/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#include "HIDPlugin.h"
#include "HIDFactory.h"

Plugin *getPlugin(){
    return new HIDPlugin();
}


void HIDPlugin::registerComponents(shared_ptr<mwComponentRegistry> registry) {
	registry->registerFactory(std::string("iodevice/usbhid"),
							  (ComponentFactory *)(new mHIDFactory()));
}
