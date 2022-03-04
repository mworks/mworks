/*
 *  FakeMonkeySpikeChannelFactory.h
 *  FakeMonkey
 *
 *  Created by bkennedy on 10/24/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#ifndef FAKE_MONKEY_SPIKE_CHANNEL_FACTORY_H
#define FAKE_MONKEY_SPIKE_CHANNEL_FACTORY_H

#include "MWorksCore/ComponentFactory.h"
#include "MWorksCore/ComponentRegistry.h"
using namespace mw;

class mFakeMonkeySpikeChannelFactory : public ComponentFactory {
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												ComponentRegistry *reg);
};

#endif


