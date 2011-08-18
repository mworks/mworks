/*
 *  StandardDynamicStimulus.cpp
 *
 *  Created by Christopher Stawarz on 8/11/10.
 *  Copyright 2010 MIT. All rights reserved.
 *
 */

#include "StandardDynamicStimulus.h"


BEGIN_NAMESPACE_MW


void StandardDynamicStimulus::describeComponent(ComponentInfo &info) {
    Stimulus::describeComponent(info);
}


StandardDynamicStimulus::StandardDynamicStimulus(const ParameterValueMap &parameters) :
    Stimulus(parameters)
{
}


bool StandardDynamicStimulus::needDraw() {
    return isPlaying();
}


Datum StandardDynamicStimulus::getCurrentAnnounceDrawData() {
    Datum announceData(M_DICTIONARY, 5);
    announceData.addElement(STIM_NAME, tag);
    announceData.addElement(STIM_ACTION, STIM_ACTION_DRAW);
    announceData.addElement(STIM_TYPE, "standard_dynamic_stimulus");  
    announceData.addElement("start_time", getStartTime());  
    return announceData;
}


END_NAMESPACE_MW























