/*
 *  FakeMonkeyEyeMovementChannelFactory.cpp
 *  FakeMonkey
 *
 *  Created by bkennedy on 10/27/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#include "FakeMonkeyEyeMovementChannelFactory.h"
#include "FakeMonkeyEyeMovementChannel.h"

shared_ptr<mw::Component> mFakeMonkeyEyeMovementChannelFactory::createObject(std::map<std::string, std::string> parameters,
																		  ComponentRegistry *reg) {
	const char *EYE_H_VARIABLE = "eye_h";
	const char *EYE_V_VARIABLE = "eye_v";
	const char *UPDATE_PERIOD = "update_interval";
	const char *DATA_SAMPLING_PERIOD = "data_interval";
	
	REQUIRE_ATTRIBUTES(parameters, EYE_H_VARIABLE, EYE_V_VARIABLE, UPDATE_PERIOD, DATA_SAMPLING_PERIOD);
	
	shared_ptr<Variable> eye_h_variable = reg->getVariable(parameters.find(EYE_H_VARIABLE)->second);	
	checkAttribute(eye_h_variable, parameters.find("reference_id")->second, EYE_H_VARIABLE, parameters.find(EYE_H_VARIABLE)->second);

	shared_ptr<Variable> eye_v_variable = reg->getVariable(parameters.find(EYE_V_VARIABLE)->second);		
	checkAttribute(eye_v_variable, parameters.find("reference_id")->second, EYE_V_VARIABLE, parameters.find(EYE_V_VARIABLE)->second);

	MWorksTime update_period = reg->getNumber(parameters.find(UPDATE_PERIOD)->second);
	MWorksTime data_sampling_period = reg->getNumber(parameters.find(DATA_SAMPLING_PERIOD)->second);
	
	int samples_per_update = update_period/data_sampling_period;
	
	shared_ptr <mw::Component> new_fake_monkey_eye_channel = shared_ptr<mw::Component>(new mFakeMonkeyEyeMovementChannel(eye_h_variable, 
																												   eye_v_variable, 
																												   update_period,
																												   samples_per_update));
	return new_fake_monkey_eye_channel;
}


