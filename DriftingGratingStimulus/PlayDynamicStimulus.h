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

#include "MonkeyWorksCore/TrialBuildingBlocks.h"
#include "DynamicStimulus.h"


class mPlayDynamicStimulus : public Action {	
protected:
	shared_ptr<mDynamicStimulus> dynamic_stimulus;
	
public:
	
	mPlayDynamicStimulus(shared_ptr<mDynamicStimulus> &the_dynamic_stimulus);
	virtual bool execute();
};

#endif 
// PLAY_DYNAMIC_STIMULUS_ACTION_H_

