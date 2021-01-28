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
    explicit BaseImageStimulus(const ParameterValueMap &parameters);
    ~BaseImageStimulus();
    
protected:
    void loadMetal(MetalDisplay &display) override;
    void unloadMetal(MetalDisplay &display) override;
    void drawMetal(MetalDisplay &display) override;
    
    virtual double getAspectRatio() const = 0;
    
    id<MTLTexture> texture;
    
};


END_NAMESPACE_MW


#endif /* BaseImageStimulus_hpp */
