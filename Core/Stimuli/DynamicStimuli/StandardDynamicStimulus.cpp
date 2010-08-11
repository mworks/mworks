/*
 *  StandardDynamicStimulus.cpp
 *
 *  Created by Christopher Stawarz on 8/11/10.
 *  Copyright 2010 MIT. All rights reserved.
 *
 */

#include "StandardDynamicStimulus.h"


StandardDynamicStimulus::StandardDynamicStimulus(const std::string &tag,
                                                 shared_ptr<Scheduler> scheduler,
                                                 shared_ptr<StimulusDisplay> display,
                                                 shared_ptr<Variable> framesPerSecond) :
    DynamicStimulusDriver(scheduler, display, framesPerSecond),
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























