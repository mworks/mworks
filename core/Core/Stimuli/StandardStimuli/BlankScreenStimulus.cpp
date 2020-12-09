//
//  BlankScreenStimulus.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 12/9/20.
//

#include "BlankScreenStimulus.hpp"

#include "AppleOpenGLContextManager.hpp"
#include "ParameterValue.h"


BEGIN_NAMESPACE_MW


const std::string BlankScreen::COLOR("color");


void BlankScreen::describeComponent(ComponentInfo &info) {
    Stimulus::describeComponent(info);
    info.setSignature("stimulus/blank_screen");
    info.addParameter(COLOR, "0.5,0.5,0.5");
}


BlankScreen::BlankScreen(const ParameterValueMap &parameters) :
    Stimulus(parameters)
{
    ParsedColorTrio pct(parameters[COLOR]);
    r = registerVariable(pct.getR());
    g = registerVariable(pct.getG());
    b = registerVariable(pct.getB());
}


void BlankScreen::draw(boost::shared_ptr<StimulusDisplay> display) {
    current_r = r->getValue().getFloat();
    current_g = g->getValue().getFloat();
    current_b = b->getValue().getFloat();
    
    glFlush();
    
    auto contextManager = boost::dynamic_pointer_cast<AppleOpenGLContextManager>(OpenGLContextManager::instance());
    
    MTLRenderPassDescriptor *renderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
    renderPassDescriptor.colorAttachments[0].texture = contextManager->getCurrentFramebufferTexture(0);
    renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
    renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
    renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(current_r, current_g, current_b, 1.0);
    
    id<MTLCommandBuffer> commandBuffer = [contextManager->getView(0).commandQueue commandBuffer];
    id<MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
    [renderEncoder endEncoding];
    [commandBuffer commit];
    
    last_r = current_r;
    last_g = current_g;
    last_b = current_b;
}


Datum BlankScreen::getCurrentAnnounceDrawData() {
    Datum announceData = Stimulus::getCurrentAnnounceDrawData();
    
    announceData.addElement(STIM_TYPE, STIM_TYPE_BLANK);
    announceData.addElement(STIM_COLOR_R, last_r);
    announceData.addElement(STIM_COLOR_G, last_g);
    announceData.addElement(STIM_COLOR_B, last_b);
    
    return announceData;
}


END_NAMESPACE_MW
