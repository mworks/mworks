//
//  FrameListStimulus.cpp
//  MovieStimulusPlugin
//
//  Created by Christopher Stawarz on 8/6/12.
//
//

#include "FrameListStimulus.h"


BEGIN_NAMESPACE_MW


void FrameListStimulus::describeComponent(ComponentInfo &info) {
    BaseFrameListStimulus::describeComponent(info);
    info.setSignature("stimulus/frame_list");
    info.addParameter(STIMULUS_GROUP, false);
    info.addParameter(ENDING, false);
}


FrameListStimulus::FrameListStimulus(const ParameterValueMap &parameters) :
    BaseFrameListStimulus(parameters),
    currentFrameTime(-1),
    currentNominalFrameNumber(-1)
{ }


Datum FrameListStimulus::getCurrentAnnounceDrawData() {
    boost::mutex::scoped_lock locker(stim_lock);
    Datum announceData = BaseFrameListStimulus::getCurrentAnnounceDrawData();
    
    announceData.addElement(STIM_TYPE, "frame_list");
    
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


























