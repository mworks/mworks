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
    @autoreleasepool {
        if (loaded)
            return;
        
        load(getDisplay(display)->getMainView().device);
        
        Stimulus::load(display);
    }
}


void MetalStimulus::unload(boost::shared_ptr<StimulusDisplay> display) {
    @autoreleasepool {
        if (!loaded)
            return;
        
        unload();
        
        Stimulus::unload(display);
    }
}


void MetalStimulus::draw(boost::shared_ptr<StimulusDisplay> display) {
    @autoreleasepool {
        // Ensure that any pending OpenGL commands are committed before we start rendering
        glFlush();
        
        id<MTLCommandBuffer> commandBuffer = [getDisplay(display)->getMainView().commandQueue commandBuffer];
        
        MTLRenderPassDescriptor *renderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
        renderPassDescriptor.colorAttachments[0].texture = getDisplay(display)->getCurrentMetalFramebufferTexture();
        renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionLoad;
        renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
        
        draw(commandBuffer, renderPassDescriptor);
        
        [commandBuffer commit];
    }
}


END_NAMESPACE_MW
