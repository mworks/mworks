/*
 *  DriftingGratingStimulusPlugins.h
 *  DriftingGratingStimulusPlugins
 *
 *  Created by bkennedy on 8/14/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#include <MWorksCore/Plugin.h>
using namespace mw;

extern "C"{
    Plugin *getPlugin();
}

class DriftingGratingStimulusPlugin : public Plugin {
    
	virtual void registerComponents(shared_ptr<mw::ComponentRegistry> registry);	
};
