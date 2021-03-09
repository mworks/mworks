//
//  TextStimulus.metal
//  TextStimulus
//
//  Created by Christopher Stawarz on 3/29/21.
//  Copyright © 2021 The MWorks Project. All rights reserved.
//

#include <metal_stdlib>

using namespace metal;


struct RasterizerData {
    float4 position [[position]];
    float2 textureCoordinate;
};


vertex RasterizerData
vertexShader(uint vertexID [[vertex_id]],
             constant float4x4 &mvpMatrix [[buffer(0)]],
             constant float2 &texCoordScale [[buffer(1)]])
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
    
    RasterizerData out;
    out.position = mvpMatrix * float4(vertexPositions[vertexID], 0.0, 1.0);
    out.textureCoordinate = texCoords[vertexID] * texCoordScale;
    return out;
}


fragment float4
fragmentShader(RasterizerData in [[stage_in]],
               texture2d<float> alphaTexture [[texture(0)]],
               constant float4 &color [[buffer(0)]])
{
    // Linear filtering is the right choice here, since there's no guarantee that
    // texels will line up exactly with display pixels (e.g. the stimulus could be
    // rotated 45 degrees)
    constexpr sampler textureSampler (coord::pixel, filter::linear);
    
    float alphaSample = alphaTexture.sample(textureSampler, in.textureCoordinate).r;
    return color * alphaSample;
}
