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


END_NAMESPACE_MW
