/*
 *  HIDPlugin.h
 *  HIDPlugin
 *
 *  Created by bkennedy on 8/14/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#ifndef HID_PLUGIN_H
#define HID_PLUGIN_H

#include <MonkeyWorksCore/Plugin.h>
using namespace mw;

extern "C"{
    Plugin *getPlugin();
}

class HIDPlugin : public Plugin {
    
	virtual void registerComponents(shared_ptr<mwComponentRegistry> registry);	
};

#endif
