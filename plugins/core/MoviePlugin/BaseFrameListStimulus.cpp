//
//  BaseFrameListStimulus.cpp
//  MovieStimulusPlugin
//
//  Created by Christopher Stawarz on 8/6/12.
//
//

#include "BaseFrameListStimulus.h"


BEGIN_NAMESPACE_MW


const std::string BaseFrameListStimulus::ENDED("ended");
const std::string BaseFrameListStimulus::LOOP("loop");


void BaseFrameListStimulus::describeComponent(ComponentInfo &info) {
    StandardDynamicStimulus::describeComponent(info);
    info.addParameter(ENDED, false);
    info.addParameter(LOOP, "0");
}


BaseFrameListStimulus::BaseFrameListStimulus(const ParameterValueMap &parameters) :
    StandardDynamicStimulus(parameters),
    loop(registerVariable(parameters[LOOP]))
{
    if (!(parameters[ENDED].empty())) {
        ended = VariablePtr(parameters[ENDED]);
    }
}


void BaseFrameListStimulus::freeze(bool shouldFreeze) {
    StandardDynamicStimulus::freeze(shouldFreeze);
    for (int i = 0; i < getNumFrames(); i++) {
        getStimulusForFrame(i)->freeze(shouldFreeze);
    }
}


void BaseFrameListStimulus::load(shared_ptr<StimulusDisplay> display) {
    for (int i = 0; i < getNumFrames(); i++) {
        getStimulusForFrame(i)->load(display);
    }
}


void BaseFrameListStimulus::unload(shared_ptr<StimulusDisplay> display) {
    for (int i = 0; i < getNumFrames(); i++) {
        getStimulusForFrame(i)->unload(display);
    }
}


void BaseFrameListStimulus::drawFrame(shared_ptr<StimulusDisplay> display) {
    int frameNumber = getFrameNumber();
    int numFrames = getNumFrames();
    if (frameNumber < numFrames) {
        getStimulusForFrame(frameNumber)->draw(display);
        lastFrameDrawn = frameNumber;
    }
}


Datum BaseFrameListStimulus::getCurrentAnnounceDrawData() {
    Datum announceData = StandardDynamicStimulus::getCurrentAnnounceDrawData();
    
    announceData.addElement(LOOP, loop->getValue());
    announceData.addElement("playing", Datum(isPlaying()));
    
    int frameNumber = getFrameNumber();
    announceData.addElement("current_frame", Datum((long)frameNumber));
    
    Datum currentStimulusAnnounceData(0L);
    if ((frameNumber >= 0) && (frameNumber < getNumFrames())) {
        currentStimulusAnnounceData = getStimulusForFrame(frameNumber)->getCurrentAnnounceDrawData();
    }
    announceData.addElement("current_stimulus", currentStimulusAnnounceData);
    
    return announceData;
}


void BaseFrameListStimulus::startPlaying() {
    lastFrameDrawn = -1;
    StandardDynamicStimulus::startPlaying();
}


int BaseFrameListStimulus::getFrameNumber() {
    if (!isPlaying()) {
        return -1;
    }
    
    int frameNumber = getNominalFrameNumber();
    const int numFrames = getNumFrames();
    
    if (bool(loop->getValue())) {
        frameNumber %= numFrames;
    } else if ((frameNumber >= numFrames) && (ended != NULL) && (ended->getValue().getInteger() == 0)) {
        ended->setValue(true);
    }
    
    return frameNumber;
}


END_NAMESPACE_MW


























