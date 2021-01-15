//
//  BaseImageStimulus.metal
//  MWorksCore
//
//  Created by Christopher Stawarz on 1/15/21.
//

#include <metal_stdlib>

using namespace metal;


static constant float2 texCoords[] =
{
    { 0.0f, 1.0f },
    { 1.0f, 1.0f },
    { 0.0f, 0.0f },
    { 1.0f, 0.0f },
};


struct RasterizerData {
    float4 position [[position]];
    float2 textureCoordinate;
};


vertex RasterizerData
BaseImageStimulus_vertexShader(uint vertexID [[vertex_id]],
                               constant float2 *vertexPositions [[buffer(0)]],
                               constant float4x4 &mvpMatrix [[buffer(1)]])
{
    RasterizerData out;
    out.position = mvpMatrix * float4(vertexPositions[vertexID], 0.0, 1.0);
    out.textureCoordinate = texCoords[vertexID];
    return out;
}


fragment float4
BaseImageStimulus_fragmentShader(RasterizerData in [[stage_in]],
                                 texture2d<float> imageTexture [[texture(0)]],
                                 constant float &alpha [[buffer(0)]])
{
    constexpr sampler textureSampler (mag_filter::linear, min_filter::linear, mip_filter::linear);
    float4 colorSample = imageTexture.sample(textureSampler, in.textureCoordinate);
    colorSample.a *= alpha;
    return colorSample;
}
