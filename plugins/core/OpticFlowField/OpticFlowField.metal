//
//  OpticFlowField.metal
//  OpticFlowField
//
//  Created by Christopher Stawarz on 6/2/23.
//

#include <metal_stdlib>

using namespace metal;


struct RasterizerData {
    float4 position [[position]];
    float pointSize [[point_size]];
    bool isHidden;
};


vertex RasterizerData
vertexShader(uint vertexID [[vertex_id]],
             const device float4 *vertexPositions [[buffer(0)]],
             constant float4x4 &sdpMatrix [[buffer(1)]],
             constant float4x4 &mvpMatrix [[buffer(2)]],
             constant float &pointSize [[buffer(3)]])
{
    RasterizerData out;
    
    // Apply scale, direction, and perspective
    float4 projectedPosition = sdpMatrix * vertexPositions[vertexID];
    projectedPosition /= projectedPosition.w;
    
    // Output the projected x/y position transformed by the MVP matrix
    out.position = mvpMatrix * float4(projectedPosition.xy, 0.0, 1.0);
    
    // Copy the point size unchanged
    out.pointSize = pointSize;
    
    // Points that lie outside the viewing frustum aren't visible.  Set
    // a flag so we know not to generate fragments for them.
    out.isHidden = !((projectedPosition.x >= -1.0) *
                     (projectedPosition.x <=  1.0) *
                     (projectedPosition.y >= -1.0) *
                     (projectedPosition.y <=  1.0) *
                     (projectedPosition.z >=  0.0) *
                     (projectedPosition.z <=  1.0));
    
    return out;
}


fragment float4
fragmentShader(RasterizerData in [[stage_in]],
               float2 pointCoord [[point_coord]],
               constant float4 &color [[buffer(0)]])
{
    if (in.isHidden) {
        discard_fragment();
    }
    
    //
    // Make the dots round, not square
    //
    // For an explanation of the edge-smoothing technique used here, see either of the following:
    // https://rubendv.be/posts/fwidth/
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
