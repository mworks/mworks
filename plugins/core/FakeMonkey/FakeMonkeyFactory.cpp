/*
 *  FakeMonkeyFactory.cpp
 *  FakeMonkeyPlugin
 *
 *  Created by labuser on 8/18/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#include "FakeMonkey.h"
#include "FakeMonkeyFactory.h"
#include "MWorksCore/Scheduler.h"

shared_ptr<mw::Component> mFakeMonkeyFactory::createObject(std::map<std::string, std::string> parameters,
												 ComponentRegistry *reg) {
	shared_ptr <Scheduler> scheduler = Scheduler::instance();

	if(scheduler == 0) {
		throw SimpleException("Trying to create a fake monkey with no scheduler");
	}
	
	const char *SPIKE_RATE = "spike_rate";
	
	shared_ptr <Variable> spike_rate;
	
	if(parameters.find(SPIKE_RATE) != parameters.end()) {
		spike_rate = reg->getVariable(parameters.find(SPIKE_RATE)->second);	
		checkAttribute(spike_rate, parameters.find("reference_id")->second, SPIKE_RATE, parameters.find(SPIKE_RATE)->second);
	} else {
		spike_rate = shared_ptr<Variable>(new ConstantVariable(25.0));
	}
	
	shared_ptr <mw::Component> new_fake_monkey = shared_ptr<mw::Component>(new mFakeMonkey(scheduler, spike_rate));
	return new_fake_monkey;
}
