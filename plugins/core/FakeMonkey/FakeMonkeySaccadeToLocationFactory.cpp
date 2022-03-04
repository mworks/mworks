/*
 *  FakeMonkeySaccadeToLocationFactory.cpp
 *  FakeMonkey
 *
 *  Created by bkennedy on 10/8/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#include "MWorksCore/ComponentRegistry.h"
#include "FakeMonkeySaccadeToLocationFactory.h"
#include "FakeMonkey.h"
#include "FakeMonkeySaccadeToLocation.h"

shared_ptr<mw::Component> mFakeMonkeySaccadeToLocationFactory::createObject(std::map<std::string, std::string> parameters,
																		 ComponentRegistry *reg) {
	REQUIRE_ATTRIBUTES(parameters, "fake_monkey", "h", "v");
	
	shared_ptr<Variable> h = reg->getVariable(parameters.find("h")->second);	
	shared_ptr<Variable> v = reg->getVariable(parameters.find("v")->second);
	
	// TODO (maybe) there's a few accepted entries here in the current parser
	shared_ptr<mFakeMonkey> fm = reg->getObject<mFakeMonkey>(parameters.find("fake_monkey")->second);
	
	checkAttribute(h, parameters.find("reference_id")->second, "h", parameters.find("h")->second);		
	
	checkAttribute(v, parameters.find("reference_id")->second, "v", parameters.find("v")->second);		
	
	checkAttribute(fm, parameters.find("reference_id")->second, "fake_monkey", parameters.find("fake_monkey")->second);		
	
	
	shared_ptr <mw::Component> newFakeMonkeySaccadeToLocationAction = shared_ptr<mw::Component>(new mFakeMonkeySaccadeToLocation(fm, 
																														   h, 
																														   v));
	return newFakeMonkeySaccadeToLocationAction;		
}

