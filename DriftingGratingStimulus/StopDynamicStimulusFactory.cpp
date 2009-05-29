/*
 *  StopDynamicStimulusFactory.cpp
 *  DriftingGratingStimulusPlugin
 *
 *  Created by bkennedy on 11/14/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#include "StopDynamicStimulusFactory.h"
#include "StopDynamicStimulus.h"
using namespace mw;

shared_ptr<mw::Component> mStopDynamicStimulusFactory::createObject(std::map<std::string, std::string> parameters,
																 mwComponentRegistry *reg) {
	
	const char *DYNAMIC_STIMULUS = "stimulus";
	
	REQUIRE_ATTRIBUTES(parameters, DYNAMIC_STIMULUS);
	
	shared_ptr <mDynamicStimulus> the_dynamic_stimulus = reg->getObject<mDynamicStimulus>(parameters.find(DYNAMIC_STIMULUS)->second);
	
	if(the_dynamic_stimulus == 0) {
		throw MissingReferenceException(parameters.find("reference_id")->second, DYNAMIC_STIMULUS, parameters.find(DYNAMIC_STIMULUS)->second);		
	}
	
	shared_ptr <mStopDynamicStimulus> new_stop_dynamic_stimulus_action = shared_ptr<mStopDynamicStimulus>(new mStopDynamicStimulus(the_dynamic_stimulus));
	return new_stop_dynamic_stimulus_action;		
}
