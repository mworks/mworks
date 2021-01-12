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
    lock_guard lock(mutex);
    
    if (loaded)
        return;
    
    @autoreleasepool {
        loadMetal(getMetalDisplay(display));
    }
    
    Stimulus::load(display);
}


void MetalStimulus::unload(boost::shared_ptr<StimulusDisplay> display) {
    lock_guard lock(mutex);
    
    if (!loaded)
        return;
    
    @autoreleasepool {
        unloadMetal(getMetalDisplay(display));
    }
    
    Stimulus::unload(display);
}


void MetalStimulus::draw(boost::shared_ptr<StimulusDisplay> display) {
    lock_guard lock(mutex);
    
    @autoreleasepool {
        drawMetal(getMetalDisplay(display));
    }
}


id<MTLLibrary> MetalStimulus::loadDefaultLibrary(MetalDisplay &display, NSBundle *bundle) {
    NSError *error = nil;
    auto library = [display.getMetalDevice() newDefaultLibraryWithBundle:bundle error:&error];
    if (!library) {
        throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN,
                              "Cannot load Metal library",
                              error.localizedDescription.UTF8String);
    }
    return library;
}


id<MTLFunction> MetalStimulus::loadShaderFunction(id<MTLLibrary> library, const std::string &name) {
    auto function = [library newFunctionWithName:@(name.c_str())];
    if (!function) {
        throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, boost::format("Cannot load Metal shader function %1%") % name);
    }
    return function;
}


id<MTLRenderPipelineState>
MetalStimulus::createRenderPipelineState(MetalDisplay &display,
                                         MTLRenderPipelineDescriptor *renderPipelineDescriptor)
{
    NSError *error = nil;
    auto renderPipelineState = [display.getMetalDevice() newRenderPipelineStateWithDescriptor:renderPipelineDescriptor
                                                                                        error:&error];
    if (!renderPipelineState) {
        throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN,
                              "Cannot create Metal render pipeline state",
                              error.localizedDescription.UTF8String);
    }
    return renderPipelineState;
}


END_NAMESPACE_MW
