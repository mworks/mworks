//
//  RenderActions.cpp
//  RenderActions
//
//  Created by Christopher Stawarz on 3/19/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#include "RenderActions.hpp"


BEGIN_NAMESPACE_MW


const std::string RenderActions::ELAPSED_TIME("elapsed_time");


void RenderActions::describeComponent(ComponentInfo &info) {
    StandardDynamicStimulus::describeComponent(info);
    
    info.setSignature("stimulus/render_actions");
    
    info.addParameter(ELAPSED_TIME, false);
    info.addParameter(AUTOPLAY, "YES");  // Change autoplay's default to YES
}


RenderActions::RenderActions(const ParameterValueMap &parameters) :
    StandardDynamicStimulus(parameters),
    elapsedTime(optionalVariable(parameters[ELAPSED_TIME]))
{ }


void RenderActions::addChild(std::map<std::string, std::string> parameters,
                             ComponentRegistryPtr reg,
                             boost::shared_ptr<Component> child)
{
    auto action = boost::dynamic_pointer_cast<Action>(child);
    if (!action) {
        throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Child component must be an action");
    }
    actions.push_back(action);
}


Datum RenderActions::getCurrentAnnounceDrawData() {
    boost::mutex::scoped_lock locker(stim_lock);
    
    auto announceData = StandardDynamicStimulus::getCurrentAnnounceDrawData();
    announceData.addElement(STIM_TYPE, "render_actions");
    return announceData;
}


void RenderActions::drawFrame(boost::shared_ptr<StimulusDisplay> display) {
    if (elapsedTime) {
        elapsedTime->setValue(Datum(getElapsedTime()));
    }
    for (auto &action : actions) {
        action->execute();
    }
}


END_NAMESPACE_MW
