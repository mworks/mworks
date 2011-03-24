/*
 *  StopDynamicStimulus.cpp
 *  DynamicStimulusPlugins
 *
 *  Created by bkennedy on 8/17/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#include "StopDynamicStimulus.h"


BEGIN_NAMESPACE(mw)


StopDynamicStimulus::StopDynamicStimulus(shared_ptr<StimulusNode> stimNode) : 
    Action(),
    stimNode(stimNode)
{
	setName("StopDynamicStimulus");
}


bool StopDynamicStimulus::execute() {	
	shared_ptr<DynamicStimulusDriver> dynamicStim(dynamic_pointer_cast<DynamicStimulusDriver>(stimNode->getStimulus()));
    
	if (!dynamicStim) {
        merror(M_PARADIGM_MESSAGE_DOMAIN, "Attempted to stop a non-dynamic stimulus.  Doing nothing.");
        return false;
	}
    
	dynamicStim->stop();
    return true;
}


END_NAMESPACE(mw)
