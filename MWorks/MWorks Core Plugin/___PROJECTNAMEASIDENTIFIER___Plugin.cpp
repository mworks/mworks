/*
 *  ___PROJECTNAME___Plugin.cpp
 *  MWPlugin
 *
 *  Created by ___FULLUSERNAME___ on ___DATE___.
 *  Copyright ___YEAR___ ___ORGANIZATIONNAME___. All rights reserved.
 *
 */

#include "___PROJECTNAME___Plugin.h"
#include "___PROJECTNAME___Factory.h"
#include "MWorksCore/ComponentFactory.h"
using namespace mw;

Plugin *getPlugin(){
    return new ___PROJECTNAME___Plugin();
}


void ___PROJECTNAME___Plugin::registerComponents(shared_ptr<mw::ComponentRegistry> registry) {
	
    // TODO: you need to customize the "signature" of the object your plugin will create
    //       The signature is of the form component/type Ð(e.g. stimulus/circle, or iodevice/NIDAQ)
    registry->registerFactory(std::string("stimulus/___PROJECTNAME___"),
							  (ComponentFactory *)(new ___PROJECTNAME___Factory()));
}

