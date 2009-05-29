/*
 *  DriftingGratingStimulusPlugins.h
 *  DriftingGratingStimulusPlugins
 *
 *  Created by bkennedy on 8/14/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#include <MonkeyWorksCore/Plugin.h>
using namespace mw;

extern "C"{
    Plugin *getPlugin();
}

class mDriftingGratingStimulusPlugin : public Plugin {
    
	virtual void registerComponents(shared_ptr<mwComponentRegistry> registry);	
};
