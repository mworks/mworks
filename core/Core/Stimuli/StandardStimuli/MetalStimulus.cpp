//
//  MetalStimulus.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 12/9/20.
//

#include "MetalStimulus.hpp"


BEGIN_NAMESPACE_MW


MetalStimulus::MetalStimulus(const ParameterValueMap &parameters) :
    Stimulus(parameters)
{ }


void MetalStimulus::load(boost::shared_ptr<StimulusDisplay> display) {
    if (loaded)
        return;
    
    @autoreleasepool {
        load(getDisplay(display));
    }
    
    Stimulus::load(display);
}


void MetalStimulus::unload(boost::shared_ptr<StimulusDisplay> display) {
    if (!loaded)
        return;
    
    @autoreleasepool {
        unload(getDisplay(display));
    }
    
    Stimulus::unload(display);
}


void MetalStimulus::draw(boost::shared_ptr<StimulusDisplay> _display) {
    // AppleStimulusDisplay invokes draw in the scope of an autorelease pool, so we
    // don't need to create one here
    
    // Ensure that any pending OpenGL commands are committed before we start rendering
    glFlush();
    
    auto display = getDisplay(_display);
    id<MTLCommandBuffer> commandBuffer = [display->getMetalCommandQueue() commandBuffer];
    draw(display, commandBuffer);
    [commandBuffer commit];
}


END_NAMESPACE_MW
