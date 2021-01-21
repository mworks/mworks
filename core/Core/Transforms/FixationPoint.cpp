//
//  FixationPoint.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 1/21/21.
//

#include "FixationPoint.hpp"


BEGIN_NAMESPACE_MW


const std::string FixationPoint::TRIGGER_WIDTH("trigger_width");
const std::string FixationPoint::TRIGGER_WATCH_X("trigger_watch_x");
const std::string FixationPoint::TRIGGER_WATCH_Y("trigger_watch_y");
const std::string FixationPoint::TRIGGER_FLAG("trigger_flag");


void FixationPoint::describeComponent(ComponentInfo &info) {
    RectangleStimulus::describeComponent(info);
    
    info.setSignature("stimulus/fixation_point");
    
    info.addParameter(TRIGGER_WIDTH);
    info.addParameter(TRIGGER_WATCH_X);
    info.addParameter(TRIGGER_WATCH_Y);
    info.addParameter(TRIGGER_FLAG);
}


FixationPoint::FixationPoint(const ParameterValueMap &parameters):
    RectangleStimulus(parameters),
    SquareRegionTrigger(registerVariable(parameters[X_POSITION]),
                        registerVariable(parameters[Y_POSITION]),
                        registerVariable(parameters[TRIGGER_WIDTH]),
                        VariablePtr(parameters[TRIGGER_WATCH_X]),
                        VariablePtr(parameters[TRIGGER_WATCH_Y]),
                        VariablePtr(parameters[TRIGGER_FLAG]))
{ }


Datum FixationPoint::getCurrentAnnounceDrawData() {
    auto announceData = RectangleStimulus::getCurrentAnnounceDrawData();
    
    announceData.addElement(STIM_TYPE, STIM_TYPE_POINT);
    
    // SquareRegionTrigger parameters
    announceData.addElement("center_x", centerx->getValue().getFloat());
    announceData.addElement("center_y", centery->getValue().getFloat());
    announceData.addElement("width", width->getValue().getFloat());
    
    return announceData;
}


ExpandableList<Datum> * FixationPoint::getGoldStandardValues() {
    // Return the center location of the fixation point
    goldStandardValues->clear();
    goldStandardValues->addElement(xoffset->getValue());
    goldStandardValues->addElement(yoffset->getValue());
    return goldStandardValues;
}


void CircularFixationPoint::describeComponent(ComponentInfo &info) {
    EllipseStimulus::describeComponent(info);
    
    info.setSignature("stimulus/circular_fixation_point");
    
    info.addParameter(FixationPoint::TRIGGER_WIDTH);
    info.addParameter(FixationPoint::TRIGGER_WATCH_X);
    info.addParameter(FixationPoint::TRIGGER_WATCH_Y);
    info.addParameter(FixationPoint::TRIGGER_FLAG);
}


CircularFixationPoint::CircularFixationPoint(const ParameterValueMap &parameters):
    EllipseStimulus(parameters),
    CircularRegionTrigger(registerVariable(parameters[X_POSITION]),
                          registerVariable(parameters[Y_POSITION]),
                          registerVariable(parameters[FixationPoint::TRIGGER_WIDTH]),
                          VariablePtr(parameters[FixationPoint::TRIGGER_WATCH_X]),
                          VariablePtr(parameters[FixationPoint::TRIGGER_WATCH_Y]),
                          VariablePtr(parameters[FixationPoint::TRIGGER_FLAG]))
{ }


Datum CircularFixationPoint::getCurrentAnnounceDrawData() {
    auto announceData = EllipseStimulus::getCurrentAnnounceDrawData();
    
    announceData.addElement(STIM_TYPE, "circular_fixation_point");
    
    // CircularRegionTrigger parameters
    announceData.addElement("center_x", centerx->getValue().getFloat());
    announceData.addElement("center_y", centery->getValue().getFloat());
    announceData.addElement("width", width->getValue().getFloat());
    
    return announceData;
}


ExpandableList<Datum> * CircularFixationPoint::getGoldStandardValues() {
    // Return the center location of the fixation point
    goldStandardValues->clear();
    goldStandardValues->addElement(xoffset->getValue());
    goldStandardValues->addElement(yoffset->getValue());
    return goldStandardValues;
}


END_NAMESPACE_MW
