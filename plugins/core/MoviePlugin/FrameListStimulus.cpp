//
//  FrameListStimulus.cpp
//  MovieStimulusPlugin
//
//  Created by Christopher Stawarz on 8/6/12.
//
//

#include "FrameListStimulus.h"


BEGIN_NAMESPACE_MW


const std::string FrameListStimulus::STIMULUS_GROUP("stimulus_group");


void FrameListStimulus::describeComponent(ComponentInfo &info) {
    BaseFrameListStimulus::describeComponent(info);
    info.setSignature("stimulus/frame_list");
    info.addParameter(ENDING, false);
    info.addParameter(STIMULUS_GROUP);
}


FrameListStimulus::FrameListStimulus(const ParameterValueMap &parameters) :
    BaseFrameListStimulus(parameters),
    stimulusGroup(parameters[STIMULUS_GROUP]),
    currentFrameTime(-1),
    currentNominalFrameNumber(-1)
{ }


Datum FrameListStimulus::getCurrentAnnounceDrawData() {
    boost::mutex::scoped_lock locker(stim_lock);
    Datum announceData = BaseFrameListStimulus::getCurrentAnnounceDrawData();
    
    announceData.addElement(STIM_TYPE, "frame_list");
    announceData.addElement(STIMULUS_GROUP, stimulusGroup->getTag());
    
    return announceData;
}


void FrameListStimulus::startPlaying() {
    currentFrameTime = -1;
    currentNominalFrameNumber = -1;
    BaseFrameListStimulus::startPlaying();
}


int FrameListStimulus::getNominalFrameNumber() {
    MWTime elapsedTime = getElapsedTime();
    if (elapsedTime != currentFrameTime) {
        currentFrameTime = elapsedTime;
        currentNominalFrameNumber++;
    }
    return currentNominalFrameNumber;
}


END_NAMESPACE_MW


























