//
//  RectangleStimulus.metal
//  MWorksCore
//
//  Created by Christopher Stawarz on 12/10/20.
//

#include <metal_stdlib>

using namespace metal;


static constant float2 vertexPositions[] =
{
    { 0.0f, 0.0f },
    { 1.0f, 0.0f },
    { 0.0f, 1.0f },
    { 1.0f, 1.0f },
};


struct RasterizerData {
    float4 position [[position]];
};


vertex RasterizerData
RectangleStimulus_vertexShader(uint vertexID [[vertex_id]],
                               constant float4x4 &mvpMatrix [[buffer(0)]])
{
    RasterizerData out;
    out.position = mvpMatrix * float4(vertexPositions[vertexID], 0.0, 1.0);
    return out;
}


fragment float4
RectangleStimulus_fragmentShader(RasterizerData in [[stage_in]],
                                 constant float4 &color [[buffer(0)]])
{
    return color;
}
