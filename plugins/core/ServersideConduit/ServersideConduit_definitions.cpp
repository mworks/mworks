// THIS IS AN AUTO-GENERATED FILE: DO NOT EDIT IT DIRECTLY
// The content of this file is pulled from the project YAML description 
// see plugin_definition.yml for details

/*
 *  ServersideConduit_definitions.cpp
 *  ServersideConduit
 *
 *
 */

#include "ServersideConduit_definitions.h"
#include "ServersideConduit.h"

namespace mw{

    void ServersideConduit::describeComponent(ComponentInfo& info){
        IODevice::describeComponent(info);
        info.setSignature("iodevice/serverside_conduit");
        info.addParameter(RESOURCE_NAME_PARAMETER, "server_conduit" );
    }

}