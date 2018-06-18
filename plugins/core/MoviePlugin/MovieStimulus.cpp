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


void MovieStimulus::describeComponent(ComponentInfo &info) {
    BaseMovieStimulus::describeComponent(info);
    info.setSignature("stimulus/movie");
    info.addParameter(STIMULUS_GROUP, false);
}


MovieStimulus::MovieStimulus(const ParameterValueMap &parameters) :
    BaseMovieStimulus(parameters)
{
    // To preserve existing behavior, do not auto-load stimulus, but *do*
    // mark it as loaded
    setDeferred(explicit_load);
    loaded = true;
}


Datum MovieStimulus::getCurrentAnnounceDrawData() {
    boost::mutex::scoped_lock locker(stim_lock);
    Datum announceData = BaseMovieStimulus::getCurrentAnnounceDrawData();
    
    announceData.addElement(STIM_TYPE, "movie");  
    
    return announceData;
}


END_NAMESPACE_MW























