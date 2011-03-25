/*
 *  PlayDynamicStimulus.cpp
 *  DynamicStimulusPlugins
 *
 *  Created by bkennedy on 8/17/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#include "PlayDynamicStimulus.h"


BEGIN_NAMESPACE_MW


PlayDynamicStimulus::PlayDynamicStimulus(shared_ptr<StimulusNode> stimNode) :
    Action(),
    stimNode(stimNode)
{
	setName("PlayDynamicStimulus");
}


bool PlayDynamicStimulus::execute() {	
	shared_ptr<DynamicStimulusDriver> dynamicStim(dynamic_pointer_cast<DynamicStimulusDriver>(stimNode->getStimulus()));

	if (!dynamicStim) {
        merror(M_PARADIGM_MESSAGE_DOMAIN, "Attempted to play a non-dynamic stimulus.  Doing nothing.");
        return false;
	}

	dynamicStim->play();
    return true;
}


END_NAMESPACE_MW
