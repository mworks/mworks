//
//  MetalStimulus.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 12/9/20.
//

#ifndef MetalStimulus_hpp
#define MetalStimulus_hpp

#include "AppleOpenGLContextManager.hpp"
#include "ParameterValue.h"
#include "Stimulus.h"


BEGIN_NAMESPACE_MW


class MetalStimulus : public Stimulus {
    
public:
    explicit MetalStimulus(const ParameterValueMap &parameters);
    
    void load(boost::shared_ptr<StimulusDisplay> display) override;
    void unload(boost::shared_ptr<StimulusDisplay> display) override;
    void draw(boost::shared_ptr<StimulusDisplay> display) override;
    
    virtual void draw(id<MTLCommandBuffer> commandBuffer, MTLRenderPassDescriptor *renderPassDescriptor) = 0;
    
private:
    boost::shared_ptr<AppleOpenGLContextManager> contextManager;
    
};


END_NAMESPACE_MW


#endif /* MetalStimulus_hpp */
