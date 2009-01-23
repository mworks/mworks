/*
 *  MSSWGamepadPlugin.h
 *  MSSWGamepadPlugin
 *
 *  Created by bkennedy on 8/14/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#ifndef MSSW_GAMEPAD_PLUGIN_H
#define MSSW_GAMEPAD_PLUGIN_H

#include <MonkeyWorksCore/Plugin.h>
using namespace mw;

extern "C"{
    Plugin *getPlugin();
}

class mMSSWGamepadPlugin : public Plugin {
    
	virtual void registerComponents(shared_ptr<mw::mwComponentRegistry> registry);	
};

#endif
