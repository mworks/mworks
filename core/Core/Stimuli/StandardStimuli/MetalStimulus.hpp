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
    
    void load(boost::shared_ptr<StimulusDisplay> display) override;
    void unload(boost::shared_ptr<StimulusDisplay> display) override;
    void draw(boost::shared_ptr<StimulusDisplay> display) override;
    
protected:
    static boost::shared_ptr<AppleStimulusDisplay> getDisplay(const boost::shared_ptr<StimulusDisplay> &display) {
        return boost::dynamic_pointer_cast<AppleStimulusDisplay>(display);
    }
    
private:
    virtual void load(id<MTLDevice> device) { }
    virtual void unload() { }
    virtual void draw(id<MTLCommandBuffer> commandBuffer, MTLRenderPassDescriptor *renderPassDescriptor) { }
    
};


END_NAMESPACE_MW


#endif /* MetalStimulus_hpp */
