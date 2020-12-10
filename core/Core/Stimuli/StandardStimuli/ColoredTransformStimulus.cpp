//
//  ColoredTransformStimulus.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 12/10/20.
//

#include "ColoredTransformStimulus.hpp"


BEGIN_NAMESPACE_MW


const std::string ColoredTransformStimulus::COLOR("color");


void ColoredTransformStimulus::describeComponent(ComponentInfo &info) {
    AlphaBlendedTransformStimulus::describeComponent(info);
    info.addParameter(COLOR, "1.0,1.0,1.0");
}


ColoredTransformStimulus::ColoredTransformStimulus(const ParameterValueMap &parameters) :
    AlphaBlendedTransformStimulus(parameters)
{
    ParsedColorTrio pct(parameters[COLOR]);
    r = registerVariable(pct.getR());
    g = registerVariable(pct.getG());
    b = registerVariable(pct.getB());
}


void ColoredTransformStimulus::draw(shared_ptr<StimulusDisplay> display) {
    current_r = r->getValue().getFloat();
    current_g = g->getValue().getFloat();
    current_b = b->getValue().getFloat();
    
    AlphaBlendedTransformStimulus::draw(display);
    
    last_r = current_r;
    last_g = current_g;
    last_b = current_b;
}


Datum ColoredTransformStimulus::getCurrentAnnounceDrawData() {
    Datum announceData = AlphaBlendedTransformStimulus::getCurrentAnnounceDrawData();
    
    announceData.addElement(STIM_COLOR_R, last_r);
    announceData.addElement(STIM_COLOR_G, last_g);
    announceData.addElement(STIM_COLOR_B, last_b);
    
    return announceData;
}


END_NAMESPACE_MW
