//
//  DriftingGratingStimulus.metal
//  DriftingGratingStimulusPlugin
//
//  Created by Christopher Stawarz on 2/1/21.
//

#include <metal_stdlib>

using namespace metal;

#include "DriftingGratingStimulusShaderTypes.h"


constant bool nonMaskGrating [[function_constant(nonMaskGratingFunctionConstantIndex)]];


struct RasterizerData {
    float4 position [[position]];
    float gratingCoordinate;
    float2 maskCoordinate [[function_constant(nonMaskGrating)]];
};


vertex RasterizerData
vertexShader(uint vertexID [[vertex_id]],
             constant float4x4 &mvpMatrix [[buffer(mvpMatrixBufferIndex)]],
             const device float *gratingCoords [[buffer(gratingCoordsBufferIndex)]],
             constant float &aspectRatio [[buffer(aspectRatioBufferIndex), function_constant(nonMaskGrating)]])
{
    constexpr float2 vertexPositions[] = {
        { -0.5f, -0.5f },
        {  0.5f, -0.5f },
        { -0.5f,  0.5f },
        {  0.5f,  0.5f }
    };
    
    RasterizerData out;
    out.position = mvpMatrix * float4(vertexPositions[vertexID], 0.0, 1.0);
    out.gratingCoordinate = gratingCoords[vertexID];
    if (nonMaskGrating) {
        float mask_x_offset = 0.0;
        float mask_y_offset = 0.0;
        if (aspectRatio < 1.0) {
            mask_x_offset = 0.5 / aspectRatio - 0.5;
        } else if (aspectRatio > 1.0) {
            mask_y_offset = 0.5 * aspectRatio - 0.5;
        }
        const float2 maskCoords[] = {
            { 0.0f - mask_x_offset, 0.0f - mask_y_offset },
            { 1.0f + mask_x_offset, 0.0f - mask_y_offset },
            { 0.0f - mask_x_offset, 1.0f + mask_y_offset },
            { 1.0f + mask_x_offset, 1.0f + mask_y_offset }
        };
        out.maskCoordinate = maskCoords[vertexID];
    }
    return out;
}


fragment float4
fragmentShader(RasterizerData in [[stage_in]],
               constant GratingParameters &gratingParams [[buffer(gratingParamsBufferIndex)]],
               constant MaskParameters &maskParams [[buffer(maskParamsBufferIndex), function_constant(nonMaskGrating)]],
               constant float4 &color [[buffer(colorBufferIndex), function_constant(nonMaskGrating)]])
{
    float normGratingCoord = in.gratingCoordinate - floor(in.gratingCoordinate);
    float gratingValue;
    
    switch (gratingParams.gratingType) {
        case GratingType::sinusoid:
            gratingValue = 0.5 * (1.0 + cos(2.0 * M_PI_F * normGratingCoord));
            break;
            
        case GratingType::square: {
            //
            // This is a smoothed version of the original equation, which was
            //   gratingValue = float(cos(2.0 * M_PI_F * normGratingCoord) > 0.0);
            //
            float halfDelta = fwidth(in.gratingCoordinate) / 2.0;
            gratingValue = ((1.0 - smoothstep(0.25 - halfDelta, 0.25 + halfDelta, normGratingCoord)) +
                            smoothstep(0.75 - halfDelta, 0.75 + halfDelta, normGratingCoord));
            break;
        }
            
        case GratingType::triangle:
            gratingValue = sign(normGratingCoord - 0.5) * (normGratingCoord - 0.5) / 0.5;
            break;
            
        case GratingType::sawtooth: {
            //
            // This is a smoothed version of the original equation, which was
            //   gratingValue = normGratingCoord;
            //
            float halfDelta = fwidth(in.gratingCoordinate) / 2.0;
            gratingValue = (normGratingCoord
                            - smoothstep(1.0 - halfDelta, 1.0 + halfDelta, normGratingCoord)
                            + (1.0 - smoothstep(-halfDelta, halfDelta, normGratingCoord)));
            break;
        }
    }
    
    if (gratingParams.inverted) {
        gratingValue = 1.0 - gratingValue;
    }
    
    if (!nonMaskGrating) {
        return float4(1.0, 1.0, 1.0, gratingValue);
    }
    
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
    }
    
    maskValue *= float(in.maskCoordinate.x >= 0.0);
    maskValue *= float(in.maskCoordinate.x <= 1.0);
    maskValue *= float(in.maskCoordinate.y >= 0.0);
    maskValue *= float(in.maskCoordinate.y <= 1.0);
    
    return float4(color.rgb * gratingValue, color.a * maskValue);
}
