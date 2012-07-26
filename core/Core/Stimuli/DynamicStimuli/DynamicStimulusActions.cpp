//
//  DynamicStimulusActions.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 8/18/11.
//  Copyright 2011 MIT. All rights reserved.
//

#include "DynamicStimulusActions.h"

#include "DynamicStimulusDriver.h"


BEGIN_NAMESPACE_MW


const std::string DynamicStimulusAction::STIMULUS("stimulus");


void DynamicStimulusAction::describeComponent(ComponentInfo &info) {
    Action::describeComponent(info);
    info.addParameter(STIMULUS);
}


DynamicStimulusAction::DynamicStimulusAction(const ParameterValueMap &parameters) :
    Action(parameters),
    stimulusNode(parameters[STIMULUS])
{ }


void PlayDynamicStimulus::describeComponent(ComponentInfo &info) {
    DynamicStimulusAction::describeComponent(info);
    info.setSignature("action/play_dynamic_stimulus");
}


PlayDynamicStimulus::PlayDynamicStimulus(const ParameterValueMap &parameters) :
    DynamicStimulusAction(parameters)
{
    setName("PlayDynamicStimulus");
}


bool PlayDynamicStimulus::execute() {
    DynamicStimulusDriverPtr stimulus(getDynamicStimulus());
    
    if (!stimulus) {
        merror(M_PARADIGM_MESSAGE_DOMAIN, "Attempted to play a non-dynamic stimulus.  Doing nothing.");
        return false;
    }

    stimulus->play();
    return true;
}


void StopDynamicStimulus::describeComponent(ComponentInfo &info) {
    DynamicStimulusAction::describeComponent(info);
    info.setSignature("action/stop_dynamic_stimulus");
}


StopDynamicStimulus::StopDynamicStimulus(const ParameterValueMap &parameters) :
    DynamicStimulusAction(parameters)
{
    setName("StopDynamicStimulus");
}


bool StopDynamicStimulus::execute() {
    DynamicStimulusDriverPtr stimulus(getDynamicStimulus());
    
    if (!stimulus) {
        merror(M_PARADIGM_MESSAGE_DOMAIN, "Attempted to stop a non-dynamic stimulus.  Doing nothing.");
        return false;
    }
    
    stimulus->stop();
    return true;
}


void PauseDynamicStimulus::describeComponent(ComponentInfo &info) {
    DynamicStimulusAction::describeComponent(info);
    info.setSignature("action/pause_dynamic_stimulus");
}


PauseDynamicStimulus::PauseDynamicStimulus(const ParameterValueMap &parameters) :
    DynamicStimulusAction(parameters)
{
    setName("PauseDynamicStimulus");
}


bool PauseDynamicStimulus::execute() {
    DynamicStimulusDriverPtr stimulus(getDynamicStimulus());
    
    if (!stimulus) {
        merror(M_PARADIGM_MESSAGE_DOMAIN, "Attempted to pause a non-dynamic stimulus.  Doing nothing.");
        return false;
    }
    
    stimulus->pause();
    return true;
}


void UnpauseDynamicStimulus::describeComponent(ComponentInfo &info) {
    DynamicStimulusAction::describeComponent(info);
    info.setSignature("action/unpause_dynamic_stimulus");
}


UnpauseDynamicStimulus::UnpauseDynamicStimulus(const ParameterValueMap &parameters) :
    DynamicStimulusAction(parameters)
{
    setName("UnpauseDynamicStimulus");
}


bool UnpauseDynamicStimulus::execute() {
    DynamicStimulusDriverPtr stimulus(getDynamicStimulus());
    
    if (!stimulus) {
        merror(M_PARADIGM_MESSAGE_DOMAIN, "Attempted to unpause a non-dynamic stimulus.  Doing nothing.");
        return false;
    }
    
    stimulus->unpause();
    return true;
}


END_NAMESPACE_MW


























