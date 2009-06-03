/*
 *  ÇPROJECTNAMEÈ.cpp
 *  MWPlugin
 *
 *  Created by David Cox on 2/26/09.
 *  Copyright 2009 Harvard University. All rights reserved.
 *
 */

#include "ÇPROJECTNAMEÈPlugin.h"
#include "ÇPROJECTNAMEÈFactory.h"
#include "MonkeyWorksCore/ComponentFactory.h"
using namespace mw;

Plugin *getPlugin(){
    return new ÇPROJECTNAMEÈPlugin();
}


void ÇPROJECTNAMEÈPlugin::registerComponents(shared_ptr<mw::ComponentRegistry> registry) {
	
    // TODO: you need to customize the "signature" of the object your plugin will create
    //       The signature is of the form component/type –(e.g. stimulus/circle, or iodevice/NIDAQ)
    registry->registerFactory(std::string("stimulus/ÇPROJECTNAMEÈ"),
							  (ComponentFactory *)(new ÇPROJECTNAMEÈFactory()));
}

