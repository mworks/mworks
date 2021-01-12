//
//  MetalStimulus.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 12/9/20.
//

#ifndef MetalStimulus_hpp
#define MetalStimulus_hpp

#include "AppleStimulusDisplay.hpp"
#include "ParameterValue.h"
#include "Stimulus.h"


BEGIN_NAMESPACE_MW


class MetalStimulus : public Stimulus {
    
public:
    explicit MetalStimulus(const ParameterValueMap &parameters);
    
    RenderingMode getRenderingMode() const override { return RenderingMode::Metal; }
    
    void load(boost::shared_ptr<StimulusDisplay> display) override;
    void unload(boost::shared_ptr<StimulusDisplay> display) override;
    void draw(boost::shared_ptr<StimulusDisplay> display) override;
    
protected:
    using MetalDisplay = AppleStimulusDisplay;
    
    static id<MTLLibrary> loadDefaultLibrary(MetalDisplay &display, NSBundle *bundle);
    static id<MTLFunction> loadShaderFunction(id<MTLLibrary> library, const std::string &name);
    static id<MTLRenderPipelineState> createRenderPipelineState(MetalDisplay &display,
                                                                MTLRenderPipelineDescriptor *renderPipelineDescriptor);
    
    virtual void loadMetal(MetalDisplay &display) { }
    virtual void unloadMetal(MetalDisplay &display) { }
    virtual void drawMetal(MetalDisplay &display) { }
    
private:
    static MetalDisplay & getMetalDisplay(const boost::shared_ptr<StimulusDisplay> &display) {
        return *(boost::dynamic_pointer_cast<AppleStimulusDisplay>(display));
    }
    
    using lock_guard = std::lock_guard<std::mutex>;
    lock_guard::mutex_type mutex;
    
};


END_NAMESPACE_MW


#endif /* MetalStimulus_hpp */
