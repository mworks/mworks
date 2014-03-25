/*
 *  MovieStimulus.cpp
 *  MovieStimulusPlugin
 *
 *  Created by labuser on 5/16/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#include "MovieStimulus.h"


BEGIN_NAMESPACE_MW


const std::string MovieStimulus::STIMULUS_GROUP("stimulus_group");


void MovieStimulus::describeComponent(ComponentInfo &info) {
    BaseMovieStimulus::describeComponent(info);
    info.setSignature("stimulus/movie");
    info.addParameter(STIMULUS_GROUP);
}


MovieStimulus::MovieStimulus(const ParameterValueMap &parameters) :
    BaseMovieStimulus(parameters),
    stimulusGroup(parameters[STIMULUS_GROUP])
{
    // To preserve existing behavior, do not auto-load stimulus
    setDeferred(explicit_load);
}


Datum MovieStimulus::getCurrentAnnounceDrawData() {
    boost::mutex::scoped_lock locker(stim_lock);
    Datum announceData = BaseMovieStimulus::getCurrentAnnounceDrawData();
    
    announceData.addElement(STIM_TYPE, "movie");  
    announceData.addElement(STIMULUS_GROUP, stimulusGroup->getTag());  
    
    return announceData;
}


END_NAMESPACE_MW























