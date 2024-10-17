//
//  OrientedNoise.metal
//  OrientedNoise
//
//  Created by Christopher Stawarz on 6/5/24.
//  Copyright © 2024 The MWorks Project. All rights reserved.
//

#include <metal_stdlib>

using namespace metal;


struct RasterizerData {
    float4 position [[position]];
    float2 textureCoordinate;
};


vertex RasterizerData
vertexShader(uint vertexID [[vertex_id]],
             constant float4x4 &mvpMatrix [[buffer(0)]])
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
    out.textureCoordinate = texCoords[vertexID];
    return out;
}


fragment float4
fragmentShader(RasterizerData in [[stage_in]],
               texture2d<float> noiseTexture [[texture(0)]],
               constant float4 &color [[buffer(0)]])
{
    constexpr sampler textureSampler (filter::linear);
    float noiseValue = noiseTexture.sample(textureSampler, in.textureCoordinate).r;
    return float4(noiseValue * color.rgb, color.a);
}
