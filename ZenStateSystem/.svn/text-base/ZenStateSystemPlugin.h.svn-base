/*
 *  ZenStateSystemPlugin.h
 *  MonkeyWorksCore
 *
 *  Created by bkennedy on 10/29/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#ifndef ZEN_STATE_SYSTEM_PLUGIN_H
#define ZEN_STATE_SYSTEM_PLUGIN_H

#include "MonkeyWorksCore/Plugin.h"
#include "MonkeyWorksCore/ComponentRegistry_new.h"
using namespace mw;

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
	virtual void registerComponents(shared_ptr<mwComponentRegistry> registry);
};

#endif


