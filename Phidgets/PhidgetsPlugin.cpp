/*
 *  CoxlabCorePlugins.cpp
 *  CoxlabCorePlugins
 *
 *  Created by David Cox on 4/29/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "PhidgetsPlugin.h"
#include "Phidgets.h"

using namespace mw;

Plugin *getPlugin(){
    return new PhidgetsPlugin();
}


void PhidgetsPlugin::registerComponents(shared_ptr<ComponentRegistry> registry) {
	
	registry->registerFactory(std::string("iodevice/phidget"),
								  (ComponentFactory *)(new PhidgetDeviceFactory()));
								  
	registry->registerFactory(std::string("iochannel/phidget"),
								  (ComponentFactory *)(new PhidgetDeviceChannelFactory()));
	
}
