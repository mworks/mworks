/*
 *  PlayDynamicStimulus.cpp
 *  DynamicStimulusPlugins
 *
 *  Created by bkennedy on 8/17/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#include "PlayDynamicStimulus.h"

mPlayDynamicStimulus::mPlayDynamicStimulus(shared_ptr<mDynamicStimulus> &the_dynamic_stimulus) : Action() 
{
	setName("PlayDynamicStimulus");
	dynamic_stimulus=the_dynamic_stimulus;
}

bool mPlayDynamicStimulus::execute() {	
	dynamic_stimulus->play();
    return true;
}

