//
//  ColoredTransformStimulus.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 12/10/20.
//

#ifndef ColoredTransformStimulus_hpp
#define ColoredTransformStimulus_hpp

#include "AlphaBlendedTransformStimulus.hpp"


BEGIN_NAMESPACE_MW


class ColoredTransformStimulus : public AlphaBlendedTransformStimulus {
    
public:
    static const std::string COLOR;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit ColoredTransformStimulus(const ParameterValueMap &parameters);
    
    Datum getCurrentAnnounceDrawData() override;
    
protected:
    void drawMetal(MetalDisplay &display) override;
    
    void setCurrentColor(id<MTLRenderCommandEncoder> renderCommandEncoder, NSUInteger bufferIndex) const;
    
    VariablePtr r;
    VariablePtr g;
    VariablePtr b;
    
    float current_r, current_g, current_b;
    
};


inline void ColoredTransformStimulus::setCurrentColor(id<MTLRenderCommandEncoder> renderCommandEncoder,
                                                      NSUInteger bufferIndex) const
{
    auto currentColor = simd::make_float4(current_r, current_g, current_b, current_alpha);
    setFragmentBytes(renderCommandEncoder, currentColor, bufferIndex);
}


END_NAMESPACE_MW


#endif /* ColoredTransformStimulus_hpp */
