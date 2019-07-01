/*
 *  ZenStateSystemPlugin.cpp
 *  MWorksCore
 *
 *  Created by bkennedy on 10/29/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#include "ZenStateSystemPlugin.h"
#include "ZenStateSystemFactory.h"


BEGIN_NAMESPACE_MW


void StandardStateSystemPlugin::registerComponents(shared_ptr<ComponentRegistry> registry) {
	registry->registerFactory("ZenStateSystem", boost::make_shared<StandardStateSystemFactory>());
}	

Plugin *getPlugin(){
    return new StandardStateSystemPlugin();
}


END_NAMESPACE_MW
