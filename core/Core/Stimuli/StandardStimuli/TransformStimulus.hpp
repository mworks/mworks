//
//  TransformStimulus.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 12/10/20.
//

#ifndef TransformStimulus_hpp
#define TransformStimulus_hpp

#include "MetalStimulus.hpp"


BEGIN_NAMESPACE_MW


class TransformStimulus : public MetalStimulus {
    
public:
    static const std::string X_SIZE;
    static const std::string Y_SIZE;
    static const std::string X_POSITION;
    static const std::string Y_POSITION;
    static const std::string ROTATION;
    static const std::string FULLSCREEN;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit TransformStimulus(const ParameterValueMap &parameters);
    ~TransformStimulus();
    
    Datum getCurrentAnnounceDrawData() override;
    
protected:
    void unloadMetal(MetalDisplay &display) override;
    void drawMetal(MetalDisplay &display) override;
    
    void getCurrentSize(float &sizeX, float &sizeY) const;
    
    MTLRenderPipelineDescriptor * createRenderPipelineDescriptor(MetalDisplay &display,
                                                                 id<MTLFunction> vertexFunction,
                                                                 id<MTLFunction> fragmentFunction) const;
    virtual void configureBlending(MTLRenderPipelineColorAttachmentDescriptor *colorAttachment) const { }
    
    void setCurrentMVPMatrix(MetalDisplay &display,
                             id<MTLRenderCommandEncoder> renderCommandEncoder,
                             NSUInteger bufferIndex) const;
    virtual simd::float4x4 getCurrentMVPMatrix(const simd::float4x4 &projectionMatrix) const;
    
    const VariablePtr xoffset;
    const VariablePtr yoffset;
    const VariablePtr xscale;
    const VariablePtr yscale;
    const VariablePtr rotation;
    const bool fullscreen;
    
    id<MTLRenderPipelineState> renderPipelineState;
    
    float current_posx, current_posy, current_sizex, current_sizey, current_rot;
    
};


inline void TransformStimulus::setCurrentMVPMatrix(MetalDisplay &display,
                                                   id<MTLRenderCommandEncoder> renderCommandEncoder,
                                                   NSUInteger bufferIndex) const
{
    auto currentMVPMatrix = getCurrentMVPMatrix(display.getMetalProjectionMatrix());
    setVertexBytes(renderCommandEncoder, currentMVPMatrix, bufferIndex);
}


END_NAMESPACE_MW


#endif /* TransformStimulus_hpp */
