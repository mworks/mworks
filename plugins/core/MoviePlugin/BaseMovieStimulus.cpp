//
//  BaseMovieStimulus.cpp
//  MovieStimulusPlugin
//
//  Created by Christopher Stawarz on 8/6/12.
//
//

#include "BaseMovieStimulus.h"


BEGIN_NAMESPACE_MW


const std::string BaseMovieStimulus::FRAMES_PER_SECOND("frames_per_second");


void BaseMovieStimulus::describeComponent(ComponentInfo &info) {
    BaseFrameListStimulus::describeComponent(info);
    info.addParameter(FRAMES_PER_SECOND);
}


BaseMovieStimulus::BaseMovieStimulus(const ParameterValueMap &parameters) :
    BaseFrameListStimulus(parameters),
    framesPerSecond(registerVariable(parameters[FRAMES_PER_SECOND]))
{ }


void BaseMovieStimulus::startPlaying() {
    const double frameRate = framesPerSecond->getValue().getFloat();
    const double refreshRate = StimulusDisplay::getCurrentStimulusDisplay()->getMainDisplayRefreshRate();
    
    if ((frameRate > refreshRate) || (fmod(refreshRate, frameRate) != 0.0)) {
        mwarning(M_DISPLAY_MESSAGE_DOMAIN,
                 "Requested frame rate (%g) is incompatible with display refresh rate (%g)",
                 frameRate,
                 refreshRate);
        
    }
    
    framesPerUS = frameRate / 1.0e6;
    
    BaseFrameListStimulus::startPlaying();
}


bool BaseMovieStimulus::needDraw() {
    if (!BaseFrameListStimulus::needDraw()) {
        return false;
    }
    
    const int frameNumber = getFrameNumber();
    const int numFrames = getNumFrames();
    
    if (frameNumber == getLastFrameDrawn()) {
        // Here we account for the case of a one-frame movie set to loop.  Since such a movie makes sense
        // only if the sole frame is being updated dynamically, we always redraw, so that any updates
        // become visible.
        return (numFrames == 1);
    }
    
    return (frameNumber <= numFrames);
}


Datum BaseMovieStimulus::getCurrentAnnounceDrawData() {
    Datum announceData = BaseFrameListStimulus::getCurrentAnnounceDrawData();
    announceData.addElement(FRAMES_PER_SECOND, framesPerSecond->getValue().getInteger());
    return announceData;
}


END_NAMESPACE_MW



























