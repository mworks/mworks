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
    using VertexPositionArray = std::array<simd::float2, 4>;
    
    static VertexPositionArray makeVertexPositionArray(float x0, float y0,
                                                       float x1, float y1,
                                                       float x2, float y2,
                                                       float x3, float y3);
    static VertexPositionArray getVertexPositions(double aspectRatio);
    
    explicit BaseImageStimulus(const ParameterValueMap &parameters);
    ~BaseImageStimulus();
    
protected:
    void loadMetal(MetalDisplay &display) override;
    void unloadMetal(MetalDisplay &display) override;
    void drawMetal(MetalDisplay &display) override;
    
    void setVertexPositions(MetalDisplay &display, id<MTLBlitCommandEncoder> blitCommandEncoder, double aspectRatio);
    
    id<MTLBuffer> vertexPositionsBuffer;
    id<MTLTexture> texture;
    
};


inline auto BaseImageStimulus::makeVertexPositionArray(float x0, float y0,
                                                       float x1, float y1,
                                                       float x2, float y2,
                                                       float x3, float y3) -> VertexPositionArray
{
    return VertexPositionArray {
        simd::make_float2(x0, y0),
        simd::make_float2(x1, y1),
        simd::make_float2(x2, y2),
        simd::make_float2(x3, y3)
    };
}


END_NAMESPACE_MW


#endif /* BaseImageStimulus_hpp */
