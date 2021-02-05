//
//  BaseImageStimulus.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 1/15/21.
//

#ifndef BaseImageStimulus_hpp
#define BaseImageStimulus_hpp

#include "AlphaBlendedTransformStimulus.hpp"


BEGIN_NAMESPACE_MW


class BaseImageStimulus : public AlphaBlendedTransformStimulus {
    
public:
    using AlphaBlendedTransformStimulus::AlphaBlendedTransformStimulus;
    
protected:
    void loadMetal(MetalDisplay &display) override;
    void drawMetal(MetalDisplay &display) override;
    
    virtual bool prepareCurrentTexture(MetalDisplay &display) { return true; }
    
    virtual double getCurrentAspectRatio() const = 0;
    virtual id<MTLTexture> getCurrentTexture() const = 0;
    
};


END_NAMESPACE_MW


#endif /* BaseImageStimulus_hpp */
