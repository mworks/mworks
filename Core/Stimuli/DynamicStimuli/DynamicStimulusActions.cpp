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


void DynamicStimulusAction::finalize(std::map<std::string, std::string> parameters, ComponentRegistryPtr reg) {
    Action::finalize(parameters, reg);
    
    stimulus = boost::dynamic_pointer_cast<DynamicStimulusDriver>(stimulusNode->getStimulus());
    
    if (!stimulus) {
        throw SimpleException("Action target is not a dynamic stimulus", stimulusNode->getStimulus()->getTag());
    }

}


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
    stimulus->stop();
    return true;
}


END_NAMESPACE_MW


























