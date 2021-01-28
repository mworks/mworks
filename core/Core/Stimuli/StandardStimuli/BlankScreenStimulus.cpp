//
//  BlankScreenStimulus.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 12/9/20.
//

#include "BlankScreenStimulus.hpp"


BEGIN_NAMESPACE_MW


const std::string BlankScreenStimulus::COLOR("color");


void BlankScreenStimulus::describeComponent(ComponentInfo &info) {
    MetalStimulus::describeComponent(info);
    info.setSignature("stimulus/blank_screen");
    info.addParameter(COLOR, "0.5,0.5,0.5");
}


BlankScreenStimulus::BlankScreenStimulus(const ParameterValueMap &parameters) :
    MetalStimulus(parameters)
{
    ParsedColorTrio pct(parameters[COLOR]);
    r = registerVariable(pct.getR());
    g = registerVariable(pct.getG());
    b = registerVariable(pct.getB());
}


Datum BlankScreenStimulus::getCurrentAnnounceDrawData() {
    auto announceData = MetalStimulus::getCurrentAnnounceDrawData();
    
    announceData.addElement(STIM_TYPE, STIM_TYPE_BLANK);
    announceData.addElement(STIM_COLOR_R, current_r);
    announceData.addElement(STIM_COLOR_G, current_g);
    announceData.addElement(STIM_COLOR_B, current_b);
    
    return announceData;
}


void BlankScreenStimulus::drawMetal(MetalDisplay &display) {
    current_r = r->getValue().getFloat();
    current_g = g->getValue().getFloat();
    current_b = b->getValue().getFloat();
    
    auto renderPassDescriptor = display.createMetalRenderPassDescriptor(MTLLoadActionClear);
    renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(current_r, current_g, current_b, 1.0);
    auto renderCommandEncoder = createRenderCommandEncoder(display, renderPassDescriptor);
    [renderCommandEncoder endEncoding];
}


END_NAMESPACE_MW
