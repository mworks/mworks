/*
 *  FakeMonkeyJuiceChannelFactory.cpp
 *  FakeMonkey
 *
 *  Created by bkennedy on 10/24/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#include "FakeMonkeyJuiceChannelFactory.h"
#include "FakeMonkeyJuiceChannel.h"

shared_ptr<mw::Component> mFakeMonkeyJuiceChannelFactory::createObject(std::map<std::string, std::string> parameters,
																	ComponentRegistry *reg) {
	const char *JUICE_VARIABLE = "variable";
	
	REQUIRE_ATTRIBUTES(parameters, JUICE_VARIABLE);
	
	shared_ptr<Variable> juice_variable = reg->getVariable(parameters.find(JUICE_VARIABLE)->second);	
	
	shared_ptr <mw::Component> new_fake_monkey_juice_channel = shared_ptr<mw::Component>(new mFakeMonkeyJuiceChannel(juice_variable));
	return new_fake_monkey_juice_channel;
}

