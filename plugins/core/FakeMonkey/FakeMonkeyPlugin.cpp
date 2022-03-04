/*
 *  FakeMonkeyPlugin.cpp
 *  FakeMonkeyPlugin
 *
 *  Created by bkennedy on 8/14/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#include "FakeMonkeyPlugin.h"
#include "FakeMonkeyFactory.h"
#include "FakeMonkeySaccadeToLocationFactory.h"
#include "FakeMonkeySaccadeAndFixateFactory.h"
#include "FakeMonkeyFixateFactory.h"
#include "FakeMonkeyJuiceChannelFactory.h"
#include "FakeMonkeySpikeChannelFactory.h"
#include "FakeMonkeyEyeMovementChannelFactory.h"

MW_SYMBOL_PUBLIC
Plugin *getPlugin(){
    return new mFakeMonkeyPlugin();
}


void mFakeMonkeyPlugin::registerComponents(shared_ptr<ComponentRegistry> registry) {
	registry->registerFactory(std::string("iochannel/spike"),
							  (ComponentFactory *)(new mFakeMonkeySpikeChannelFactory()));
	
	registry->registerFactory(std::string("iochannel/juice"),
							  (ComponentFactory *)(new mFakeMonkeyJuiceChannelFactory()));
	
	registry->registerFactory(std::string("iochannel/eye_movement"),
							  (ComponentFactory *)(new mFakeMonkeyEyeMovementChannelFactory()));
	
	registry->registerFactory(std::string("iodevice/fake_monkey"),
							  (ComponentFactory *)(new mFakeMonkeyFactory()));
	
	registry->registerFactory(std::string("action/fake_monkey_saccade_to_location"),
							  (ComponentFactory *)(new mFakeMonkeySaccadeToLocationFactory()));
	
	registry->registerFactory(std::string("action/fake_monkey_saccade_and_fixate"),
							  (ComponentFactory *)(new mFakeMonkeySaccadeAndFixateFactory()));
	
	registry->registerFactory(std::string("action/fake_monkey_fixate"),
							  (ComponentFactory *)(new mFakeMonkeyFixateFactory()));
}
