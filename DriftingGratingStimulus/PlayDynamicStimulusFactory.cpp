/*
 *  PlayDynamicStimulusFactory.cpp
 *  DriftingGratingStimulusPlugin
 *
 *  Created by bkennedy on 11/14/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#include "PlayDynamicStimulusFactory.h"
#include "PlayDynamicStimulus.h"
using namespace mw;

shared_ptr<mw::Component> mPlayDynamicStimulusFactory::createObject(std::map<std::string, std::string> parameters,
													   mwComponentRegistry *reg) {
	
	const char *DYNAMIC_STIMULUS = "stimulus";
	
	REQUIRE_ATTRIBUTES(parameters, DYNAMIC_STIMULUS);
	
	shared_ptr <mDynamicStimulus> the_dynamic_stimulus = reg->getObject<mDynamicStimulus>(parameters.find(DYNAMIC_STIMULUS)->second);
	
	if(the_dynamic_stimulus == 0) {
		throw MissingReferenceException(parameters.find("reference_id")->second, DYNAMIC_STIMULUS, parameters.find(DYNAMIC_STIMULUS)->second);		
	}
	
	shared_ptr <mPlayDynamicStimulus> new_play_dynamic_stimulus_action = shared_ptr<mPlayDynamicStimulus>(new mPlayDynamicStimulus(the_dynamic_stimulus));
	return new_play_dynamic_stimulus_action;		
}
