/*
 *  RandomizeBackground.cpp
 *  WhiteNoiseBackground
 *
 *  Created by Christopher Stawarz on 12/17/10.
 *  Copyright 2010 MIT. All rights reserved.
 *
 */

#include "RandomizeBackground.h"

#include "WhiteNoiseBackground.h"


BEGIN_NAMESPACE_MW


static const std::string STIMULUS("stimulus");


void RandomizeBackground::describeComponent(ComponentInfo &info) {
    Action::describeComponent(info);
    info.setSignature("action/randomize_background");
    info.addParameter(STIMULUS);
}


RandomizeBackground::RandomizeBackground(const ParameterValueMap &parameters) :
    Action(parameters),
    backgroundNode(parameters[STIMULUS])
{
    setName("RandomizeBackground");
}


bool RandomizeBackground::execute() {
    auto background = boost::dynamic_pointer_cast<WhiteNoiseBackground>(backgroundNode->getStimulus());
    
    if (!background) {
        merror(M_PARADIGM_MESSAGE_DOMAIN, "Invalid target for randomize_background.  Doing nothing.");
        return false;
    }
    
    background->randomize();
    return true;
}


END_NAMESPACE_MW
