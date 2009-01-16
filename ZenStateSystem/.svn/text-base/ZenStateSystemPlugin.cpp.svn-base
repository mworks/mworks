/*
 *  ZenStateSystemPlugin.cpp
 *  MonkeyWorksCore
 *
 *  Created by bkennedy on 10/29/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#include "ZenStateSystemPlugin.h"
#include "ZenStateSystemFactory.h"

void StandardStateSystemPlugin::registerComponents(shared_ptr<mwComponentRegistry> registry) {
	registry->registerFactory(std::string("ZenStateSystem"),
							  (ComponentFactory *)(new StandardStateSystemFactory()));
}	

Plugin *getPlugin(){
    return new StandardStateSystemPlugin();
}

