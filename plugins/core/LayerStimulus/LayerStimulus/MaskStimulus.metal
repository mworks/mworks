//
//  MaskStimulus.metal
//  LayerStimulus
//
//  Created by Christopher Stawarz on 1/26/21.
//  Copyright © 2021 The MWorks Project. All rights reserved.
//

#include <metal_stdlib>

using namespace metal;

#include "MaskStimulusShaderTypes.h"


struct RasterizerData {
    float4 position [[position]];
    float2 maskCoordinate;
};


vertex RasterizerData
MaskStimulus_vertexShader(uint vertexID [[vertex_id]],
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
    out.maskCoordinate = vertexPositions[vertexID];
    return out;
}


fragment float4
MaskStimulus_fragmentShader(RasterizerData in [[stage_in]],
                            constant MaskParameters &maskParams [[buffer(0)]])
{
    constexpr float2 maskCenter = { 0.5f, 0.5f };
    constexpr float maskRadius = 0.5f;
    float maskValue = 1.0;
    
    switch (maskParams.maskType) {
        case MaskType::rectangle:
            break;
            
        case MaskType::ellipse: {
            //
            // For an explanation of the edge-smoothing technique used here, see either of the following:
            // https://rubendv.be/blog/opengl/drawing-antialiased-circles-in-opengl/
            // http://www.numb3r23.net/2015/08/17/using-fwidth-for-distance-based-anti-aliasing/
            //
            float dist = distance(in.maskCoordinate, maskCenter);
            float delta = fwidth(dist);
            maskValue = 1.0 - smoothstep(maskRadius - delta, maskRadius, dist);
            break;
        }
            
        case MaskType::gaussian: {
            float dist = distance(in.maskCoordinate, maskCenter) / maskRadius;
            maskValue = (dist - maskParams.mean) / maskParams.stdDev;
            maskValue = exp(-0.5 * maskValue * maskValue);
            if (maskParams.normalized) {
                maskValue /= maskParams.stdDev * sqrt(2.0 * M_PI_F);
                maskValue = saturate(maskValue);  // Clamp to [0,1]
            }
            break;
        }
            
        case MaskType::raisedCosine: {
            //
            // The equation used here derives from the definition of the raised-cosine filter at
            // https://en.wikipedia.org/wiki/Raised-cosine_filter
            //
            
            float edgeMax = maskRadius;
            float edgeMin = edgeMax - maskParams.edgeWidth;
            float dist = distance(in.maskCoordinate, maskCenter);
            
            maskValue = 0.5 * (1.0 + cos(M_PI_F / maskParams.edgeWidth * (dist - edgeMin)));
            maskValue *= float(dist > edgeMin);
            maskValue *= float(dist <= edgeMax);
            maskValue += float(dist <= edgeMin);
            
            break;
        }
    }
    
    if (maskParams.inverted) {
        maskValue = 1.0 - maskValue;
    }
    
    return float4(1.0, 1.0, 1.0, maskValue);
}
