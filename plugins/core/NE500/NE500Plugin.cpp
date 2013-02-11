/*
 *  CoxlabCorePlugins.cpp
 *  CoxlabCorePlugins
 *
 *  Created by David Cox on 4/29/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "NE500Plugin.h"
#include "NE500.h"

using namespace mw;

MW_SYMBOL_PUBLIC
Plugin *getPlugin(){
    return new NE500Plugin();
}


void NE500Plugin::registerComponents(shared_ptr<ComponentRegistry> registry) {
	
	registry->registerFactory(std::string("iodevice/ne500"),
							  (ComponentFactory *)(new NE500DeviceFactory()));
							  
	registry->registerFactory(std::string("iochannel/ne500"),
							  (ComponentFactory *)(new NE500DeviceChannelFactory()));
                            
}
