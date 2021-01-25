//
//  LayerStimulus.metal
//  LayerStimulus
//
//  Created by Christopher Stawarz on 1/25/21.
//  Copyright © 2021 The MWorks Project. All rights reserved.
//

#include <metal_stdlib>

using namespace metal;


static constant float2 vertexPositions[] =
{
    { -1.0f, -1.0f },
    {  1.0f, -1.0f },
    { -1.0f,  1.0f },
    {  1.0f,  1.0f },
};


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
LayerStimulus_vertexShader(uint vertexID [[vertex_id]])
{
    RasterizerData out;
    out.position = float4(vertexPositions[vertexID], 0.0, 1.0);
    out.textureCoordinate = texCoords[vertexID];
    return out;
}


fragment float4
LayerStimulus_fragmentShader(RasterizerData in [[stage_in]],
                             texture2d<float> colorTexture [[texture(0)]])
{
    constexpr sampler textureSampler (mag_filter::linear, min_filter::linear);
    return colorTexture.sample(textureSampler, in.textureCoordinate);
}
