//
//  BlankScreenStimulus.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 12/9/20.
//

#include "BlankScreenStimulus.hpp"


BEGIN_NAMESPACE_MW


const std::string BlankScreen::COLOR("color");


void BlankScreen::describeComponent(ComponentInfo &info) {
    MetalStimulus::describeComponent(info);
    info.setSignature("stimulus/blank_screen");
    info.addParameter(COLOR, "0.5,0.5,0.5");
}


BlankScreen::BlankScreen(const ParameterValueMap &parameters) :
    MetalStimulus(parameters)
{
    ParsedColorTrio pct(parameters[COLOR]);
    r = registerVariable(pct.getR());
    g = registerVariable(pct.getG());
    b = registerVariable(pct.getB());
}


Datum BlankScreen::getCurrentAnnounceDrawData() {
    Datum announceData = MetalStimulus::getCurrentAnnounceDrawData();
    
    announceData.addElement(STIM_TYPE, STIM_TYPE_BLANK);
    announceData.addElement(STIM_COLOR_R, last_r);
    announceData.addElement(STIM_COLOR_G, last_g);
    announceData.addElement(STIM_COLOR_B, last_b);
    
    return announceData;
}


void BlankScreen::draw(id<MTLCommandBuffer> commandBuffer, MTLRenderPassDescriptor *renderPassDescriptor) {
    current_r = r->getValue().getFloat();
    current_g = g->getValue().getFloat();
    current_b = b->getValue().getFloat();
    
    renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
    renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(current_r, current_g, current_b, 1.0);
    
    id<MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
    [renderEncoder endEncoding];
    
    last_r = current_r;
    last_g = current_g;
    last_b = current_b;
}


END_NAMESPACE_MW
