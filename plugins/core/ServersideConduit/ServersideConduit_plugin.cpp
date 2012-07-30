// THIS IS AN AUTO-GENERATED FILE: DO NOT EDIT IT DIRECTLY
// The content of this file is pulled from the project YAML description 
// see plugin_definition.yml for details

/*
 *  ServersideConduitPlugin.cpp
 *  ServersideConduit
 *
 *
 */

#include "ServersideConduit.h"
#include "ServersideConduit_plugin.h"
#include "MWorksCore/ComponentFactory.h"
#include "MWorksCore/StandardComponentFactory.h"
#include "ServersideConduit_definitions.h"


Plugin *getPlugin(){
    return new ServersideConduitPlugin();
}


using namespace mw;

void ServersideConduitPlugin::registerComponents(shared_ptr<ComponentRegistry> registry) {
	
    registry->registerFactory<StandardComponentFactory, ServersideConduit>();
}

