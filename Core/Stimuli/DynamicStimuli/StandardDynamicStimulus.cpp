/*
 *  StandardDynamicStimulus.cpp
 *
 *  Created by Christopher Stawarz on 8/11/10.
 *  Copyright 2010 MIT. All rights reserved.
 *
 */

#include "StandardDynamicStimulus.h"


BEGIN_NAMESPACE_MW


static const std::string FRAMES_PER_SECOND("frames_per_second");


void StandardDynamicStimulus::describeComponent(ComponentInfo &info) {
    Stimulus::describeComponent(info);
    info.addParameter(FRAMES_PER_SECOND, "refreshRate()");
}


StandardDynamicStimulus::StandardDynamicStimulus(const std::string &tag, shared_ptr<Variable> framesPerSecond) :
    Stimulus(tag),
    DynamicStimulusDriver(shared_ptr<Scheduler>(), registerVariable(framesPerSecond)),
    lastFrameDrawn(-1)
{
}


StandardDynamicStimulus::StandardDynamicStimulus(const ParameterValueMap &parameters) :
    Stimulus(parameters),
    DynamicStimulusDriver(shared_ptr<Scheduler>(), registerVariable(parameters[FRAMES_PER_SECOND])),
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
    Datum announceData(M_DICTIONARY, 5);
    announceData.addElement(STIM_NAME, tag);
    announceData.addElement(STIM_ACTION, STIM_ACTION_DRAW);
    announceData.addElement(STIM_TYPE, "standard_dynamic_stimulus");  
    announceData.addElement("frames_per_second", frames_per_second->getValue().getInteger());  
    announceData.addElement("start_time", start_time);  
    return announceData;
}


END_NAMESPACE_MW























