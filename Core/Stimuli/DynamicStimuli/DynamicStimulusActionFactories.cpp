/*
 *  PlayDynamicStimulusFactory.cpp
 *  DriftingGratingStimulusPlugin
 *
 *  Created by bkennedy on 11/14/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#include "DynamicStimulusActionFactories.h"
#include "PlayDynamicStimulus.h"
#include "StopDynamicStimulus.h"
using namespace mw;

shared_ptr<mw::Component> PlayDynamicStimulusFactory::createObject(std::map<std::string, std::string> parameters,
                                                                    mw::ComponentRegistry *reg) {
	
	const char *DYNAMIC_STIMULUS = "stimulus";
	
	REQUIRE_ATTRIBUTES(parameters, DYNAMIC_STIMULUS);
	
	shared_ptr <DynamicStimulusDriver> the_dynamic_stimulus = reg->getObject<DynamicStimulusDriver>(parameters.find(DYNAMIC_STIMULUS)->second);
	
	if(the_dynamic_stimulus == 0) {
		throw MissingReferenceException(parameters.find("reference_id")->second, DYNAMIC_STIMULUS, parameters.find(DYNAMIC_STIMULUS)->second);		
	}
	
	shared_ptr <PlayDynamicStimulus> new_play_dynamic_stimulus_action = shared_ptr<PlayDynamicStimulus>(new PlayDynamicStimulus(the_dynamic_stimulus));
	return new_play_dynamic_stimulus_action;		
}

shared_ptr<mw::Component> StopDynamicStimulusFactory::createObject(std::map<std::string, std::string> parameters,
                                                                    mw::ComponentRegistry *reg) {
	
	const char *DYNAMIC_STIMULUS = "stimulus";
	
	REQUIRE_ATTRIBUTES(parameters, DYNAMIC_STIMULUS);
	
	shared_ptr <DynamicStimulusDriver> the_dynamic_stimulus = reg->getObject<DynamicStimulusDriver>(parameters.find(DYNAMIC_STIMULUS)->second);
	
	if(the_dynamic_stimulus == 0) {
		throw MissingReferenceException(parameters.find("reference_id")->second, DYNAMIC_STIMULUS, parameters.find(DYNAMIC_STIMULUS)->second);		
	}
	
	shared_ptr <StopDynamicStimulus> new_stop_dynamic_stimulus_action = shared_ptr<StopDynamicStimulus>(new StopDynamicStimulus(the_dynamic_stimulus));
	return new_stop_dynamic_stimulus_action;		
}


