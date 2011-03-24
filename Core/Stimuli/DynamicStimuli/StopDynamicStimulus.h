/*
 *  StopDynamicStimulus.h
 *  DynamicStimulusPlugins
 *
 *  Created by bkennedy on 8/17/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#ifndef STOP_DYNAMIC_STIMULUS_ACTION_H_
#define STOP_DYNAMIC_STIMULUS_ACTION_H_

#include "TrialBuildingBlocks.h"
#include "DynamicStimulusDriver.h"


BEGIN_NAMESPACE(mw)


class StopDynamicStimulus : public Action {	
protected:
	shared_ptr<StimulusNode> stimNode;
public:
	StopDynamicStimulus(shared_ptr<StimulusNode> stimNode);
	virtual bool execute();
};


END_NAMESPACE(mw)


#endif 
// STOP_DYNAMIC_STIMULUS_ACTION_H_

