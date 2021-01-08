//
//  BaseFrameListStimulus.cpp
//  MovieStimulusPlugin
//
//  Created by Christopher Stawarz on 8/6/12.
//
//

#include "BaseFrameListStimulus.h"


BEGIN_NAMESPACE_MW


const std::string BaseFrameListStimulus::STIMULUS_GROUP("stimulus_group");
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
    repeats(registerVariable(parameters[REPEATS])),
    lastFrameDrawn(-1),
    didSetEnding(false),
    didSetEnded(false)
{
    // We need to use find() because "stimulus_group" isn't a valid parameter for all
    // BaseFrameListStimulus subclasses, meaning it won't always have a default value
    if ((parameters.find(STIMULUS_GROUP) != parameters.end()) && !(parameters[STIMULUS_GROUP].empty())) {
        stimulusGroup = StimulusGroupPtr(parameters[STIMULUS_GROUP]);
    }
    
    // Ditto for "ending"
    if ((parameters.find(ENDING) != parameters.end()) && !(parameters[ENDING].empty())) {
        ending = VariablePtr(parameters[ENDING]);
    }
    
    if (!(parameters[ENDED].empty())) {
        ended = VariablePtr(parameters[ENDED]);
    }
}


void BaseFrameListStimulus::addChild(std::map<std::string, std::string> parameters,
                                     ComponentRegistryPtr reg,
                                     boost::shared_ptr<Component> child)
{
    auto stim = boost::dynamic_pointer_cast<Stimulus>(child);
    if (!stim) {
        throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Child component must be a stimulus");
    }
    if (stimulusGroup) {
        throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN,
                              "Child stimuli are not allowed when a stimulus group is specified");
    }
    frames.push_back(stim);
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
    loaded = true;
}


void BaseFrameListStimulus::unload(shared_ptr<StimulusDisplay> display) {
    for (int i = 0; i < getNumFrames(); i++) {
        getStimulusForFrame(i)->unload(display);
    }
    loaded = false;
}


void BaseFrameListStimulus::drawFrame(shared_ptr<StimulusDisplay> display) {
    int frameNumber = getFrameNumber();
    int numFrames = getNumFrames();
    if (frameNumber < numFrames) {
        auto stim = getStimulusForFrame(frameNumber);
        if (stim->isLoaded()) {
            display->setRenderingMode(stim->getRenderingMode());
            stim->draw(display);
        } else {
            merror(M_DISPLAY_MESSAGE_DOMAIN,
                   "Stimulus \"%s\" (frame number %d of stimulus \"%s\") is not loaded and will not be displayed",
                   stim->getTag().c_str(),
                   frameNumber,
                   getTag().c_str());
        }
        lastFrameDrawn = frameNumber;
    }
}


Datum BaseFrameListStimulus::getCurrentAnnounceDrawData() {
    Datum announceData = StandardDynamicStimulus::getCurrentAnnounceDrawData();
    
    if (stimulusGroup) {
        announceData.addElement(STIMULUS_GROUP, stimulusGroup->getTag());
    }
    
    announceData.addElement(LOOP, loop->getValue());
    announceData.addElement("playing", Datum(isPlaying()));
    
    int frameNumber = getFrameNumber();
    announceData.addElement("current_frame", Datum((long)frameNumber));
    
    Datum currentStimulusAnnounceData(0L);
    if ((frameNumber >= 0) && (frameNumber < getNumFrames())) {
        auto stim = getStimulusForFrame(frameNumber);
        if (stim->isLoaded()) {
            currentStimulusAnnounceData = stim->getCurrentAnnounceDrawData();
        }
    }
    announceData.addElement("current_stimulus", currentStimulusAnnounceData);
    
    return announceData;
}


void BaseFrameListStimulus::startPlaying() {
    lastFrameDrawn = -1;
    didSetEnding = false;
    didSetEnded = false;
    StandardDynamicStimulus::startPlaying();
}


int BaseFrameListStimulus::getNumFrames() {
    if (stimulusGroup) {
        return stimulusGroup->getNElements();
    }
    return frames.size();
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
    
    //
    // This method can be called multiple times during the drawing cycle.  Therefore, we use
    // didSetEnding and didSetEnded to ensure that ending and ended are set only once during
    // a given play-through.
    //
    if ((frameNumber == numFrames - 1) && (ending != NULL) && (ending->getValue().getInteger() == 0)) {
        if (!didSetEnding) {
            ending->setValue(true);
            didSetEnding = true;
        }
    } else if ((frameNumber >= numFrames) && (ended != NULL) && (ended->getValue().getInteger() == 0)) {
        if (!didSetEnded) {
            ended->setValue(true);
            didSetEnded = true;
        }
    }
    
    return frameNumber;
}


shared_ptr<Stimulus> BaseFrameListStimulus::getStimulusForFrame(int frameNumber) {
    if (stimulusGroup) {
        return stimulusGroup->getElement(frameNumber);
    }
    return frames.at(frameNumber);
}


END_NAMESPACE_MW
