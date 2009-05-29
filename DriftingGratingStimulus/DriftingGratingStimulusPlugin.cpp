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
#include "PlayDynamicStimulusFactory.h"
#include "StopDynamicStimulusFactory.h"
#include "MonkeyWorksCore/ComponentFactory.h"
using namespace mw;

Plugin *getPlugin(){
    return new mDriftingGratingStimulusPlugin();
}


void mDriftingGratingStimulusPlugin::registerComponents(shared_ptr<mwComponentRegistry> registry) {
	registry->registerFactory(std::string("action/play_dynamic_stimulus"),
							  (ComponentFactory *)(new mPlayDynamicStimulusFactory()));
	
	registry->registerFactory(std::string("action/stop_dynamic_stimulus"),
							  (ComponentFactory *)(new mStopDynamicStimulusFactory()));
	
	registry->registerFactory(std::string("stimulus/drifting_grating"),
							  (ComponentFactory *)(new mDriftingGratingStimulusFactory()));
}
