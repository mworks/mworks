/*
 *  ZenStateSystemPlugin.h
 *  MWorksCore
 *
 *  Created by bkennedy on 10/29/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#ifndef ZEN_STATE_SYSTEM_PLUGIN_H
#define ZEN_STATE_SYSTEM_PLUGIN_H

#include "MWorksCore/Plugin.h"
#include "MWorksCore/ComponentRegistry.h"


BEGIN_NAMESPACE_MW


///////////////////////////////////////////////////////////////////////////////////////
// Plugin entry point
//
// The extern "C" bit ensures that the name of this function is not mangled (C++
// requires name-mangling in order to achieve function overriding.  Our plugin host
// needs to know what name to look for, and for this plugin architecture, that name
// is 'getPlugin'
///////////////////////////////////////////////////////////////////////////////////////

extern "C"{
    Plugin *getPlugin();
}

class StandardStateSystemPlugin : public Plugin {
	virtual void registerComponents(shared_ptr<ComponentRegistry> registry);
};


END_NAMESPACE_MW


#endif


