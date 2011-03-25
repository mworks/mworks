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


BEGIN_NAMESPACE_MW


class PlayDynamicStimulus : public Action {	
protected:
	shared_ptr<StimulusNode> stimNode;
	
public:
	
	PlayDynamicStimulus(shared_ptr<StimulusNode> stimNode);
	virtual bool execute();
};


END_NAMESPACE_MW


#endif 
// PLAY_DYNAMIC_STIMULUS_ACTION_H_

