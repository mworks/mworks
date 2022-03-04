/*
 *  FakeMonkeyFixateFactory.cpp
 *  FakeMonkey
 *
 *  Created by bkennedy on 10/8/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#include "MWorksCore/ComponentRegistry.h"
#include "FakeMonkeyFixateFactory.h"
#include "FakeMonkey.h"
#include "FakeMonkeyFixate.h"

shared_ptr<mw::Component> mFakeMonkeyFixateFactory::createObject(std::map<std::string, std::string> parameters,
															  ComponentRegistry *reg) {
	const char *DURATION = "duration";
	const char *FAKE_MONKEY = "fake_monkey";
	
	REQUIRE_ATTRIBUTES(parameters, FAKE_MONKEY, DURATION);
	
	shared_ptr<Variable> duration = reg->getVariable(parameters.find(DURATION)->second);	
	
	// TODO (maybe) there's a few accepted entries here in the current parser
	shared_ptr<mFakeMonkey> fm = reg->getObject<mFakeMonkey>(parameters.find(FAKE_MONKEY)->second);
	
	checkAttribute(duration, parameters.find("reference_id")->second, DURATION, parameters.find(DURATION)->second);		
	checkAttribute(fm, parameters.find("reference_id")->second, FAKE_MONKEY, parameters.find(FAKE_MONKEY)->second);		
	
	
	shared_ptr <mw::Component> new_fixate_action = shared_ptr<mw::Component>(new mFakeMonkeyFixate(fm,
																							 duration));
	return new_fixate_action;		
}


