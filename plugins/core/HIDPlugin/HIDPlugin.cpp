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

MW_SYMBOL_PUBLIC
Plugin *getPlugin(){
    return new HIDPlugin();
}


void HIDPlugin::registerComponents(shared_ptr<ComponentRegistry> registry) {
	registry->registerFactory(std::string("iodevice/usbhid"),
							  (ComponentFactory *)(new mHIDFactory()));
}
