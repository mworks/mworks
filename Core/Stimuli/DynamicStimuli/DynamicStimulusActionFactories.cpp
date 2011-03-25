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


BEGIN_NAMESPACE_MW


shared_ptr<mw::Component> PlayDynamicStimulusFactory::createObject(std::map<std::string, std::string> parameters,
                                                                    mw::ComponentRegistry *reg) {
	
	const char *DYNAMIC_STIMULUS = "stimulus";
	
	REQUIRE_ATTRIBUTES(parameters, DYNAMIC_STIMULUS);
	
	shared_ptr<StimulusNode> stimNode = reg->getStimulus(parameters.find(DYNAMIC_STIMULUS)->second);
    CHECK_ATTRIBUTE(stimNode, parameters, DYNAMIC_STIMULUS);
	
	shared_ptr <PlayDynamicStimulus> new_play_dynamic_stimulus_action = shared_ptr<PlayDynamicStimulus>(new PlayDynamicStimulus(stimNode));
	return new_play_dynamic_stimulus_action;		
}

shared_ptr<mw::Component> StopDynamicStimulusFactory::createObject(std::map<std::string, std::string> parameters,
                                                                    mw::ComponentRegistry *reg) {
	
	const char *DYNAMIC_STIMULUS = "stimulus";
	
	REQUIRE_ATTRIBUTES(parameters, DYNAMIC_STIMULUS);
	
	shared_ptr<StimulusNode> stimNode = reg->getStimulus(parameters.find(DYNAMIC_STIMULUS)->second);
    CHECK_ATTRIBUTE(stimNode, parameters, DYNAMIC_STIMULUS);
	
	shared_ptr <StopDynamicStimulus> new_stop_dynamic_stimulus_action = shared_ptr<StopDynamicStimulus>(new StopDynamicStimulus(stimNode));
	return new_stop_dynamic_stimulus_action;		
}


END_NAMESPACE_MW
