/*
 *  FakeMonkeySaccadeAndFixateFactory.cpp
 *  FakeMonkey
 *
 *  Created by labuser on 10/8/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#include "MWorksCore/ComponentRegistry.h"
#include "FakeMonkeySaccadeAndFixateFactory.h"
#include "FakeMonkey.h"
#include "FakeMonkeySaccadeAndFixate.h"

shared_ptr<mw::Component> mFakeMonkeySaccadeAndFixateFactory::createObject(std::map<std::string, std::string> parameters,
																		ComponentRegistry *reg) {
	const char *H = "h";
	const char *V = "v";
	const char *DURATION = "duration";
	const char *FAKE_MONKEY = "fake_monkey";
	
	REQUIRE_ATTRIBUTES(parameters, FAKE_MONKEY, DURATION, H, V);
	
	shared_ptr<Variable> h = reg->getVariable(parameters.find(H)->second);	
	shared_ptr<Variable> v = reg->getVariable(parameters.find(V)->second);
	shared_ptr<Variable> duration = reg->getVariable(parameters.find(DURATION)->second);	
	shared_ptr<mFakeMonkey> fm = reg->getObject<mFakeMonkey>(parameters.find(FAKE_MONKEY)->second);
	
	// TODO (maybe) there's a few accepted entries here in the current parser
	checkAttribute(h, parameters.find("reference_id")->second, H, parameters.find(H)->second);			
	checkAttribute(v, parameters.find("reference_id")->second, V, parameters.find(V)->second);		
	checkAttribute(duration, parameters.find("reference_id")->second, DURATION, parameters.find(DURATION)->second);		
	checkAttribute(fm, parameters.find("reference_id")->second, FAKE_MONKEY, parameters.find(FAKE_MONKEY)->second);		
	
	
	shared_ptr <mw::Component> new_action = shared_ptr<mw::Component>(new mFakeMonkeySaccadeAndFixate(fm, 
																								h,
																								v,
																								duration));
	return new_action;		
}
