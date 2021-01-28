//
//  EllipseStimulus.metal
//  MWorksCore
//
//  Created by Christopher Stawarz on 1/14/21.
//

#include <metal_stdlib>

using namespace metal;


struct RasterizerData {
    float4 position [[position]];
    float2 rawPosition;
};


vertex RasterizerData
EllipseStimulus_vertexShader(uint vertexID [[vertex_id]],
                             constant float4x4 &mvpMatrix [[buffer(0)]])
{
    constexpr float2 vertexPositions[] = {
        { 0.0f, 0.0f },
        { 1.0f, 0.0f },
        { 0.0f, 1.0f },
        { 1.0f, 1.0f }
    };
    
    RasterizerData out;
    out.position = mvpMatrix * float4(vertexPositions[vertexID], 0.0, 1.0);
    out.rawPosition = vertexPositions[vertexID];
    return out;
}


fragment float4
EllipseStimulus_fragmentShader(RasterizerData in [[stage_in]],
                               constant float4 &color [[buffer(0)]])
{
    //
    // For an explanation of the edge-smoothing technique used here, see either of the following:
    // https://rubendv.be/blog/opengl/drawing-antialiased-circles-in-opengl/
    // http://www.numb3r23.net/2015/08/17/using-fwidth-for-distance-based-anti-aliasing/
    //
    constexpr float2 center = { 0.5f, 0.5f };
    constexpr float radius = 0.5f;
    float dist = distance(in.rawPosition, center);
    float delta = fwidth(dist);
    float alpha = 1.0 - smoothstep(radius - delta, radius, dist);
    return float4(color.rgb, color.a * alpha);
}
