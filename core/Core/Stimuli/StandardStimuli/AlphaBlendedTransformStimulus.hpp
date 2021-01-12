//
//  AlphaBlendedTransformStimulus.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 12/10/20.
//

#ifndef AlphaBlendedTransformStimulus_hpp
#define AlphaBlendedTransformStimulus_hpp

#include "TransformStimulus.hpp"


BEGIN_NAMESPACE_MW


class AlphaBlendedTransformStimulus : public TransformStimulus {
    
public:
    static const std::string ALPHA_MULTIPLIER;
    static const std::string SOURCE_BLEND_FACTOR;
    static const std::string DEST_BLEND_FACTOR;
    static const std::string SOURCE_ALPHA_BLEND_FACTOR;
    static const std::string DEST_ALPHA_BLEND_FACTOR;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit AlphaBlendedTransformStimulus(const ParameterValueMap &parameters);
    
    Datum getCurrentAnnounceDrawData() override;
    
protected:
    static MTLBlendFactor blendFactorFromName(const std::string &name);
    
    void loadMetal(MetalDisplay &display) override;
    void drawMetal(MetalDisplay &display) override;
    
    MTLRenderPipelineDescriptor * createRenderPipelineDescriptor(MetalDisplay &display,
                                                                 id<MTLFunction> vertexFunction,
                                                                 id<MTLFunction> fragmentFunction) const override;
    
    const VariablePtr alpha_multiplier;
    const VariablePtr sourceBlendFactorName;
    const VariablePtr destBlendFactorName;
    const VariablePtr sourceAlphaBlendFactorName;
    const VariablePtr destAlphaBlendFactorName;
    
    float current_alpha;
    std::string current_source_blend_factor_name;
    MTLBlendFactor current_source_blend_factor;
    std::string current_dest_blend_factor_name;
    MTLBlendFactor current_dest_blend_factor;
    std::string current_source_alpha_blend_factor_name;
    MTLBlendFactor current_source_alpha_blend_factor;
    std::string current_dest_alpha_blend_factor_name;
    MTLBlendFactor current_dest_alpha_blend_factor;
    
};


END_NAMESPACE_MW


#endif /* AlphaBlendedTransformStimulus_hpp */
