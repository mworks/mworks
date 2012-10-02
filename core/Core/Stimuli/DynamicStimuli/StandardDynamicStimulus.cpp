/*
 *  StandardDynamicStimulus.cpp
 *
 *  Created by Christopher Stawarz on 8/11/10.
 *  Copyright 2010 MIT. All rights reserved.
 *
 */

#include "StandardDynamicStimulus.h"


BEGIN_NAMESPACE_MW


const std::string StandardDynamicStimulus::AUTOPLAY("autoplay");


void StandardDynamicStimulus::describeComponent(ComponentInfo &info) {
    Stimulus::describeComponent(info);
    info.addParameter(AUTOPLAY, "0");
}


StandardDynamicStimulus::StandardDynamicStimulus(const ParameterValueMap &parameters) :
    Stimulus(parameters),
    autoplay(parameters[AUTOPLAY]),
    didDrawWhilePaused(false)
{
}


bool StandardDynamicStimulus::needDraw() {
    return isPlaying() && !(isPaused() && didDrawWhilePaused);
}


void StandardDynamicStimulus::draw(shared_ptr<StimulusDisplay> display) {
    boost::mutex::scoped_lock locker(stim_lock);
    
    if (!isPlaying()) {
        if (autoplay->getValue().getBool()) {
            startPlaying();
        } else {
            return;
        }
    }
    
    drawFrame(display);
    
    if (isPaused()) {
        didDrawWhilePaused = true;
    }
}


Datum StandardDynamicStimulus::getCurrentAnnounceDrawData() {
    Datum announceData(M_DICTIONARY, 4);
    announceData.addElement(STIM_NAME, getTag());
    announceData.addElement(STIM_ACTION, STIM_ACTION_DRAW);
    announceData.addElement(STIM_TYPE, "standard_dynamic_stimulus");  
    announceData.addElement("start_time", getStartTime());  
    return announceData;
}


void StandardDynamicStimulus::beginPause() {
    DynamicStimulusDriver::beginPause();
    didDrawWhilePaused = false;
}


END_NAMESPACE_MW























