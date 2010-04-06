/*
 *  DriftingGratingStimulusPlugins.cpp
 *  DriftingGratingStimulusPlugins
 *
 *  Created by bkennedy on 8/14/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#include "DriftingGratingStimulusPlugin.h"
#include "DriftingGratingStimulusFactory.h"
#include "MWorksCore/ComponentFactory.h"
using namespace mw;

Plugin *getPlugin(){
    return new DriftingGratingStimulusPlugin();
}


void DriftingGratingStimulusPlugin::registerComponents(shared_ptr<mw::ComponentRegistry> registry) {
	
	registry->registerFactory(std::string("stimulus/drifting_grating"),
							  (ComponentFactory *)(new DriftingGratingStimulusFactory()));
}
