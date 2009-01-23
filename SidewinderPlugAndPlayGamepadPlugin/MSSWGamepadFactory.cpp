/*
 *  MSSWGamepadFactory.cpp
 *  MSSWGamepadPlugin
 *
 *  Created by labuser on 8/18/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#include "MSSWGamepadFactory.h"
#include "MSSWGamepad.h"

boost::shared_ptr<mw::Component> mMSSWGamepadFactory::createObject(std::map<std::string, std::string> parameters,
																	  mw::mwComponentRegistry *reg) {
	
	const char *TAG = "tag";
	const char *BUTTON_A = "a";
	const char *BUTTON_B = "b";
	const char *BUTTON_X = "x";
	const char *BUTTON_Y = "y";
	const char *TRIGGER_LEFT = "trigger_left";
	const char *TRIGGER_RIGHT = "trigger_right";
	const char *UP = "up";
	const char *DOWN = "down";
	const char *LEFT = "left";
	const char *RIGHT = "right";
	const char *UPDATE_PERIOD = "data_interval";
	const char *DATA_TYPE = "data_type";
	
	REQUIRE_ATTRIBUTES(parameters, 
					   TAG,
					   UPDATE_PERIOD, 
					   DATA_TYPE);
		
	MonkeyWorksTime update_period = reg->getNumber(parameters.find(UPDATE_PERIOD)->second);	
	
	mMSSWGamepadDataType data_type = M_UNKNOWN_DATA_TYPE;
	if(parameters.find(DATA_TYPE)->second == "event") {
		data_type = M_EVENT_DRIVEN;
	} else if(parameters.find(DATA_TYPE)->second == "continuous") {
		data_type = M_CONTINUOUS;		
	} else {
		throw SimpleException("Trying to create MSSWGamepad with illegal \"data_type\"");
	}
	
	boost::shared_ptr<mw::Variable> button_a = boost::shared_ptr<mw::Variable>(new mw::ConstantVariable(Data(M_INTEGER, 0)));	
	if(parameters.find(BUTTON_A) != parameters.end()) {
		button_a = reg->getVariable(parameters.find(BUTTON_A)->second);	
		checkAttribute(button_a, 
					   parameters.find("reference_id")->second, 
					   BUTTON_A, 
					   parameters.find(BUTTON_A)->second);
	}
	
	boost::shared_ptr<mw::Variable> button_b = boost::shared_ptr<mw::Variable>(new mw::ConstantVariable(Data(M_INTEGER, 0)));	
	if(parameters.find(BUTTON_B) != parameters.end()) {
		button_b = reg->getVariable(parameters.find(BUTTON_B)->second);	
		checkAttribute(button_b, 
					   parameters.find("reference_id")->second, 
					   BUTTON_B, 
					   parameters.find(BUTTON_B)->second);
	}
	
	boost::shared_ptr<mw::Variable> button_x = boost::shared_ptr<mw::Variable>(new mw::ConstantVariable(Data(M_INTEGER, 0)));	
	if(parameters.find(BUTTON_X) != parameters.end()) {
		button_x = reg->getVariable(parameters.find(BUTTON_X)->second);	
		checkAttribute(button_x, 
					   parameters.find("reference_id")->second, 
					   BUTTON_X, 
					   parameters.find(BUTTON_X)->second);
	}
	
	boost::shared_ptr<mw::Variable> button_y = boost::shared_ptr<mw::Variable>(new mw::ConstantVariable(Data(M_INTEGER, 0)));	
	if(parameters.find(BUTTON_Y) != parameters.end()) {
		button_y = reg->getVariable(parameters.find(BUTTON_Y)->second);	
		checkAttribute(button_y, 
					   parameters.find("reference_id")->second, 
					   BUTTON_Y, 
					   parameters.find(BUTTON_Y)->second);
	}
	
	boost::shared_ptr<mw::Variable> trigger_left = boost::shared_ptr<mw::Variable>(new mw::ConstantVariable(Data(M_INTEGER, 0)));	
	if(parameters.find(TRIGGER_LEFT) != parameters.end()) {
		trigger_left = reg->getVariable(parameters.find(TRIGGER_LEFT)->second);	
		checkAttribute(trigger_left, 
					   parameters.find("reference_id")->second, 
					   TRIGGER_LEFT, 
					   parameters.find(TRIGGER_LEFT)->second);
	}
		
	boost::shared_ptr<mw::Variable> trigger_right = boost::shared_ptr<mw::Variable>(new mw::ConstantVariable(Data(M_INTEGER, 0)));	
	if(parameters.find(TRIGGER_RIGHT) != parameters.end()) {
		trigger_right = reg->getVariable(parameters.find(TRIGGER_RIGHT)->second);	
		checkAttribute(trigger_right, 
					   parameters.find("reference_id")->second, 
					   TRIGGER_RIGHT, 
					   parameters.find(TRIGGER_RIGHT)->second);
	}
		
	boost::shared_ptr<mw::Variable> up = boost::shared_ptr<mw::Variable>(new mw::ConstantVariable(Data(M_INTEGER, 0)));	
	if(parameters.find(UP) != parameters.end()) {
		up = reg->getVariable(parameters.find(UP)->second);	
		checkAttribute(up, 
					   parameters.find("reference_id")->second, 
					   UP, 
					   parameters.find(UP)->second);
	}
	
	boost::shared_ptr<mw::Variable> down = boost::shared_ptr<mw::Variable>(new mw::ConstantVariable(Data(M_INTEGER, 0)));	
	if(parameters.find(DOWN) != parameters.end()) {
		down = reg->getVariable(parameters.find(DOWN)->second);	
		checkAttribute(down, 
					   parameters.find("reference_id")->second, 
					   DOWN, 
					   parameters.find(DOWN)->second);
	}
	
	boost::shared_ptr<mw::Variable> right = boost::shared_ptr<mw::Variable>(new mw::ConstantVariable(Data(M_INTEGER, 0)));	
	if(parameters.find(RIGHT) != parameters.end()) {
		right = reg->getVariable(parameters.find(RIGHT)->second);	
		checkAttribute(right, 
					   parameters.find("reference_id")->second, 
					   RIGHT, 
					   parameters.find(RIGHT)->second);
	}
	
	boost::shared_ptr<mw::Variable> left = boost::shared_ptr<mw::Variable>(new mw::ConstantVariable(Data(M_INTEGER, 0)));	
	if(parameters.find(LEFT) != parameters.end()) {
		left = reg->getVariable(parameters.find(LEFT)->second);	
		checkAttribute(left, 
					   parameters.find("reference_id")->second, 
					   LEFT, 
					   parameters.find(LEFT)->second);
	}
	
	
	boost::shared_ptr <mw::Scheduler> scheduler = mw::Scheduler::instance(true);
	
	boost::shared_ptr <mw::Component> new_hid = boost::shared_ptr<mw::Component>(new mMSSWGamepad(scheduler,
																							data_type,
																							button_a, 
																							button_b, 
																							button_x,
																							button_y, 
																							trigger_left, 
																							trigger_right, 
																							up, 
																							down, 
																							left, 
																							right,
																							update_period));
	return new_hid;
}