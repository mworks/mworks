/*
 *  PlayDynamicStimulus.h
 *  DynamicStimulusPlugins
 *
 *  Created by bkennedy on 8/17/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#ifndef PLAY_DYNAMIC_STIMULUS_ACTION_H_
#define PLAY_DYNAMIC_STIMULUS_ACTION_H_

#include "TrialBuildingBlocks.h"
#include "DynamicStimulusDriver.h"

using namespace mw;

class PlayDynamicStimulus : public Action {	
protected:
	shared_ptr<DynamicStimulusDriver> dynamic_stimulus;
	
public:
	
	PlayDynamicStimulus(shared_ptr<DynamicStimulusDriver> the_dynamic_stimulus);
	virtual bool execute();
};




#endif 
// PLAY_DYNAMIC_STIMULUS_ACTION_H_

