/*
 *  ___PROJECTNAMEASIDENTIFIER___Plugin.cpp
 *  ___PROJECTNAME___
 *
 *  Created by ___FULLUSERNAME___ on ___DATE___.
 *  Copyright ___YEAR___ ___ORGANIZATIONNAME___. All rights reserved.
 *
 */

#include <MWorksCore/ComponentFactory.h>

#include "___PROJECTNAMEASIDENTIFIER___Plugin.h"
#include "___PROJECTNAMEASIDENTIFIER___Factory.h"

using namespace mw;


Plugin* getPlugin() {
    return new ___PROJECTNAMEASIDENTIFIER___Plugin();
}


void ___PROJECTNAMEASIDENTIFIER___Plugin::registerComponents(shared_ptr<ComponentRegistry> registry) {
    // TODO: you need to customize the "signature" of the object your plugin will create
    //       The signature is of the form component/type Ð(e.g. stimulus/circle, or iodevice/NIDAQ)
    registry->registerFactory(std::string("stimulus/___PROJECTNAMEASIDENTIFIER___"),
                              (ComponentFactory *)(new ___PROJECTNAMEASIDENTIFIER___Factory()));
}
