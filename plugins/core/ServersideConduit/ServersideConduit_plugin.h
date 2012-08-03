// THIS IS AN AUTO-GENERATED FILE: DO NOT EDIT IT DIRECTLY
// The content of this file is pulled from the project YAML description 
// see plugin_definition.yml for details

/*
 *  ServersideConduitPlugin.h
 *  ServersideConduit Plugin
 *

 *
 */
#ifndef ServersideConduit_PLUGIN_H_
#define ServersideConduit_PLUGIN_H_

#include <MWorksCore/Plugin.h>
using namespace mw;

extern "C"{
    Plugin *getPlugin();
}


namespace mw {
    
class ServersideConduitPlugin : public Plugin {
    
	virtual void registerComponents(shared_ptr<mw::ComponentRegistry> registry);	
};

}

#endif