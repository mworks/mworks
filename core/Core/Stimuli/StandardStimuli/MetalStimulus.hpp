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
    using DisplayPtr = boost::shared_ptr<AppleStimulusDisplay>;
    
private:
    static DisplayPtr getDisplay(const boost::shared_ptr<StimulusDisplay> &display) {
        return boost::dynamic_pointer_cast<AppleStimulusDisplay>(display);
    }
    
    virtual void load(const DisplayPtr &display) { }
    virtual void unload(const DisplayPtr &display) { }
    virtual void draw(const DisplayPtr &display, id<MTLCommandBuffer> commandBuffer) { }
    
};


END_NAMESPACE_MW


#endif /* MetalStimulus_hpp */
