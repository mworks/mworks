/*
 *  StandardDynamicStimulus.cpp
 *
 *  Created by Christopher Stawarz on 8/11/10.
 *  Copyright 2010 MIT. All rights reserved.
 *
 */

#include "StandardDynamicStimulus.h"


StandardDynamicStimulus::StandardDynamicStimulus(const std::string &tag, shared_ptr<Variable> framesPerSecond) :
    DynamicStimulusDriver(shared_ptr<Scheduler>(), framesPerSecond),
    Stimulus(tag),
    lastFrameDrawn(-1)
{
}


void StandardDynamicStimulus::didStop() {
    lastFrameDrawn = -1;
}


bool StandardDynamicStimulus::needDraw() {
    return started && (lastFrameDrawn != getFrameNumber());
}


void StandardDynamicStimulus::draw(shared_ptr<StimulusDisplay> display) {
    boost::mutex::scoped_lock locker(stim_lock);
    
    int currentFrame = getFrameNumber();
    if (-1 == currentFrame) {
        // Not playing
        return;
    }
    
    drawFrame(display, currentFrame);
    lastFrameDrawn = currentFrame;
}


Datum StandardDynamicStimulus::getCurrentAnnounceDrawData() {
    Datum announceData(M_DICTIONARY, 4);
    announceData.addElement(STIM_NAME, tag);
    announceData.addElement(STIM_ACTION, STIM_ACTION_DRAW);
    announceData.addElement(STIM_TYPE, "standard_dynamic_stimulus");  
    announceData.addElement("start_time", start_time);  
    return announceData;
}























