//
//  BaseImageStimulus.metal
//  MWorksCore
//
//  Created by Christopher Stawarz on 1/15/21.
//

#include <metal_stdlib>

using namespace metal;


struct RasterizerData {
    float4 position [[position]];
    float2 textureCoordinate;
};


vertex RasterizerData
BaseImageStimulus_vertexShader(uint vertexID [[vertex_id]],
                               constant float &aspectRatio [[buffer(0)]],
                               constant float4x4 &mvpMatrix [[buffer(1)]])
{
    constexpr float2 vertexPositions[] = {
        { 0.0f, 0.0f },
        { 1.0f, 0.0f },
        { 0.0f, 1.0f },
        { 1.0f, 1.0f }
    };
    
    constexpr float2 texCoords[] = {
        { 0.0f, 1.0f },
        { 1.0f, 1.0f },
        { 0.0f, 0.0f },
        { 1.0f, 0.0f }
    };
    
    float2 position = vertexPositions[vertexID];
    if (aspectRatio < 1.0) {
        position.x = position.x * aspectRatio + (0.5 - 0.5 * aspectRatio);
    } else if (aspectRatio > 1.0) {
        position.y = position.y / aspectRatio + (0.5 - 0.5 / aspectRatio);
    }
    
    RasterizerData out;
    out.position = mvpMatrix * float4(position, 0.0, 1.0);
    out.textureCoordinate = texCoords[vertexID];
    return out;
}


fragment float4
BaseImageStimulus_fragmentShader(RasterizerData in [[stage_in]],
                                 texture2d<float> imageTexture [[texture(0)]],
                                 constant float &alpha [[buffer(0)]])
{
    constexpr sampler textureSampler (filter::linear);
    float4 colorSample = imageTexture.sample(textureSampler, in.textureCoordinate);
    colorSample.a *= alpha;
    return colorSample;
}
