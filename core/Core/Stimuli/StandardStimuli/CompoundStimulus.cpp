//
//  CompoundStimulus.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 6/13/24.
//

#include "CompoundStimulus.hpp"


BEGIN_NAMESPACE_MW


void CompoundStimulus::describeComponent(ComponentInfo &info) {
    Stimulus::describeComponent(info);
    info.setSignature("stimulus/compound");
}


CompoundStimulus::CompoundStimulus(const ParameterValueMap &parameters) :
    Stimulus(parameters)
{ }


void CompoundStimulus::addChild(std::map<std::string, std::string> parameters,
                                ComponentRegistryPtr reg,
                                ComponentPtr child)
{
    auto stim = boost::dynamic_pointer_cast<Stimulus>(child);
    if (!stim) {
        throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Child component must be a stimulus");
    }
    children.push_back(stim);
}


void CompoundStimulus::freeze(bool shouldFreeze) {
    for (auto &child : children) {
        child->freeze(shouldFreeze);
    }
    Stimulus::freeze(shouldFreeze);
}


void CompoundStimulus::load(StimulusDisplayPtr display) {
    // Always load children, even if our loaded flag is true, as they may have been
    // unloaded independently
    for (auto &child : children) {
        child->load(display);
    }
    
    Stimulus::load(display);
}


void CompoundStimulus::unload(StimulusDisplayPtr display) {
    // Always unload children, even if our loaded flag is false, as they may have been
    // loaded independently
    for (auto &child : children) {
        child->unload(display);
    }
    
    Stimulus::unload(display);
}


void CompoundStimulus::setVisible(bool newVisible) {
    for (auto &child : children) {
        child->setVisible(newVisible);
    }
    Stimulus::setVisible(newVisible);
}


bool CompoundStimulus::needDraw(StimulusDisplayPtr display) {
    for (auto &child : children) {
        if (child->needDraw(display)) {
            return true;
        }
    }
    return false;
}


void CompoundStimulus::draw(StimulusDisplayPtr display) {
    for (auto &child : children) {
        if (child->isLoaded()) {
            child->draw(display);
        } else {
            merror(M_DISPLAY_MESSAGE_DOMAIN,
                   "Stimulus \"%s\" (child of stimulus \"%s\") is not loaded and will not be displayed",
                   child->getTag().c_str(),
                   getTag().c_str());
        }
    }
}


Datum CompoundStimulus::getCurrentAnnounceDrawData() {
    auto announceData = Stimulus::getCurrentAnnounceDrawData();
    
    announceData.addElement(STIM_TYPE, "compound");
    
    std::vector<Datum> childAnnounceData;
    for (auto &child : children) {
        if (child->isLoaded()) {
            childAnnounceData.emplace_back(child->getCurrentAnnounceDrawData());
        }
    }
    announceData.addElement("children", Datum(std::move(childAnnounceData)));
    
    return announceData;
}


END_NAMESPACE_MW
