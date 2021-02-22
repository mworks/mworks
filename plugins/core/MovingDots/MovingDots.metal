//
//  MovingDots.metal
//  MovingDots
//
//  Created by Christopher Stawarz on 2/25/21.
//

#include <metal_stdlib>

using namespace metal;


struct RasterizerData {
    float4 position [[position]];
    float pointSize [[point_size]];
};


vertex RasterizerData
vertexShader(uint vertexID [[vertex_id]],
             const device float2 *vertexPositions [[buffer(0)]],
             constant float4x4 &mvpMatrix [[buffer(1)]],
             constant float &pointSize [[buffer(2)]])
{
    RasterizerData out;
    out.position = mvpMatrix * float4(vertexPositions[vertexID], 0.0, 1.0);
    out.pointSize = pointSize;
    return out;
}


fragment float4
fragmentShader(RasterizerData in [[stage_in]],
               float2 pointCoord [[point_coord]],
               constant float4 &color [[buffer(0)]])
{
    //
    // Make the dots round, not square
    //
    // For an explanation of the edge-smoothing technique used here, see either of the following:
    // https://rubendv.be/blog/opengl/drawing-antialiased-circles-in-opengl/
    // http://www.numb3r23.net/2015/08/17/using-fwidth-for-distance-based-anti-aliasing/
    //
    
    constexpr float2 center = { 0.5f, 0.5f };
    constexpr float radius = 0.5f;
    
    float dist = distance(pointCoord, center);
    float delta = fwidth(dist);
    // Smooth only if delta is less than radius.  If delta is greater than or equal to radius,
    // smoothing might erase the dot entirely.
    float alpha = 1.0 - float(delta < radius) * smoothstep(radius - delta, radius, dist);
    
    return float4(color.rgb, color.a * alpha);
}
