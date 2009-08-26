/*
 *  ÇPROJECTNAMEÈPlugin.h
 *  MWPlugin
 *
 *  Created by ÇFULLUSERNAMEÈ on ÇDATEÈ.
 *  Copyright ÇYEARÈ ÇORGANIZATIONNAMEÈ. All rights reserved.
 *
 */
#ifndef ÇPROJECTNAMEÈ_PLUGIN_H_
#define ÇPROJECTNAMEÈ_PLUGIN_H_

#include <MonkeyWorksCore/Plugin.h>
using namespace mw;

extern "C"{
    Plugin *getPlugin();
}

class ÇPROJECTNAMEÈPlugin : public Plugin {
    
	virtual void registerComponents(shared_ptr<mw::ComponentRegistry> registry);	
};


#endif