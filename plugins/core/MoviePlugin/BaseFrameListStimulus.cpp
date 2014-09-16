//
//  BaseFrameListStimulus.cpp
//  MovieStimulusPlugin
//
//  Created by Christopher Stawarz on 8/6/12.
//
//

#include "BaseFrameListStimulus.h"


BEGIN_NAMESPACE_MW


const std::string BaseFrameListStimulus::ENDING("ending");
const std::string BaseFrameListStimulus::ENDED("ended");
const std::string BaseFrameListStimulus::LOOP("loop");
const std::string BaseFrameListStimulus::REPEATS("repeats");


void BaseFrameListStimulus::describeComponent(ComponentInfo &info) {
    StandardDynamicStimulus::describeComponent(info);
    info.addParameter(ENDED, false);
    info.addParameter(LOOP, "NO");
    info.addParameter(REPEATS, "0");
}


BaseFrameListStimulus::BaseFrameListStimulus(const ParameterValueMap &parameters) :
    StandardDynamicStimulus(parameters),
    loop(registerVariable(parameters[LOOP])),
    repeats(registerVariable(parameters[REPEATS]))
{
    // We need to use find() because "ending" isn't a valid parameter for all BaseFrameListStimulus
    // subclasses, meaning it won't always have a default value
    if ((parameters.find(ENDING) != parameters.end()) && !(parameters[ENDING].empty())) {
        ending = VariablePtr(parameters[ENDING]);
    }
    
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
        // We're looping, so just return the wrapped frame number, never triggering ending or ended
        return frameNumber % numFrames;
    }
    
    const int numRepeats = int(repeats->getValue());
    if ((numRepeats > 0) && (frameNumber < numFrames * numRepeats)) {
        if (frameNumber < numFrames * (numRepeats - 1)) {
            // We aren't yet in the last repetition cycle, so just return the wrapped frame number, without
            // (potentially) triggering ending or ended
            return frameNumber % numFrames;
        }
        frameNumber %= numFrames;
    }
    
    if ((frameNumber == numFrames - 1) && (ending != NULL) && (ending->getValue().getInteger() == 0)) {
        ending->setValue(true);
    } else if ((frameNumber >= numFrames) && (ended != NULL) && (ended->getValue().getInteger() == 0)) {
        ended->setValue(true);
    }
    
    return frameNumber;
}


END_NAMESPACE_MW


























