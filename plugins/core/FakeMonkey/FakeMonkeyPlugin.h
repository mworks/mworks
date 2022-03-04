/*
 *  FakeMonkeyPlugin.h
 *  FakeMonkeyPlugin
 *
 *  Created by bkennedy on 8/14/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#ifndef FAKE_MONKEY_PLUGIN_H
#define FAKE_MONKEY_PLUGIN_H

#include <MWorksCore/Plugin.h>
using namespace mw;

extern "C"{
    Plugin *getPlugin();
}

class mFakeMonkeyPlugin : public Plugin {
    
	virtual void registerComponents(shared_ptr<ComponentRegistry> registry);	
};

#endif
