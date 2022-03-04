/*
 *  FakeMonkeySpikeChannelFactory.cpp
 *  FakeMonkey
 *
 *  Created by bkennedy on 10/24/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#include "FakeMonkeySpikeChannelFactory.h"
#include "FakeMonkeySpikeChannel.h"

shared_ptr<mw::Component> mFakeMonkeySpikeChannelFactory::createObject(std::map<std::string, std::string> parameters,
																	ComponentRegistry *reg) {
	
	const char *SPIKE_VARIABLE = "variable";
	
	REQUIRE_ATTRIBUTES(parameters, SPIKE_VARIABLE);
	
	shared_ptr<Variable> spike_variable = reg->getVariable(parameters.find(SPIKE_VARIABLE)->second);	
	checkAttribute(spike_variable, parameters.find("reference_id")->second, SPIKE_VARIABLE, parameters.find(SPIKE_VARIABLE)->second);

	
	shared_ptr <mw::Component> new_fake_monkey_spike_channel = shared_ptr<mw::Component>(new mFakeMonkeySpikeChannel(spike_variable));
	return new_fake_monkey_spike_channel;
}

