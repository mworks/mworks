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
    
    //
    // Don't draw if we're not loaded.
    //
    // This check shouldn't be necessary, since StimulusDisplay and "composite" stimulus types that draw
    // other stimuli already test isLoaded() before drawing.  Unfortunately, it remains possible for the
    // stimulus to be unloaded between the calls to isLoaded() and draw().  This is most likely to happen
    // with dynamic stimuli, which redraw without an explicit display update request, but it can happen
    // with static stimuli, too, if the stimulus display is configured to redraw on every refresh.
    //
    // Note that if drawing is aborted at this point, the info returned by getCurrentAnnounceDrawData()
    // will either be out of date or just garbage.  Hopefully, calling it won't cause any other problems
    // (e.g. crashing the application), but that depends on the implementations in individual stimulus
    // classes.
    //
    if (!loaded) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "Stimulus \"%s\" is not loaded and will not be displayed", getTag().c_str());
        return;
    }
    
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


id<MTLFunction> MetalStimulus::loadShaderFunction(id<MTLLibrary> library,
                                                  const std::string &name,
                                                  MTLFunctionConstantValues *constantValues)
{
    NSError *error = nil;
    auto function = [library newFunctionWithName:@(name.c_str()) constantValues:constantValues error:&error];
    if (!function) {
        throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN,
                              boost::format("Cannot load Metal shader function %1%: %2%")
                              % name
                              % error.localizedDescription.UTF8String);
    }
    if (error) {
        mwarning(M_DISPLAY_MESSAGE_DOMAIN,
                 "Loaded Metal shader function %s with warnings: %s",
                 name.c_str(),
                 error.localizedDescription.UTF8String);
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
