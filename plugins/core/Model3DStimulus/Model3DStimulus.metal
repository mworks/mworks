//
//  Model3DStimulus.metal
//  Model3DStimulus
//
//  Created by Christopher Stawarz on 7/18/24.
//

#include <metal_stdlib>

using namespace metal;


struct VertexData {
    float3 position [[attribute(0)]];
    float3 normal [[attribute(1)]];
};


struct FragmentData {
    float4 position [[position]];
    float3 modelNDCPosition;
    float3 worldNormal;
};


vertex FragmentData
vertexShader(VertexData in [[stage_in]],
             constant float4x4 &modelMVPMatrix [[buffer(1)]],
             constant float4x4 &displayMVPMatrix [[buffer(2)]])
{
    float4 modelNDCPosition = modelMVPMatrix * float4(in.position, 1.0);
    modelNDCPosition /= modelNDCPosition.w;
    
    FragmentData out;
    out.position = displayMVPMatrix * modelNDCPosition;
    out.modelNDCPosition = modelNDCPosition.xyz;
    out.worldNormal = in.normal;
    return out;
}


fragment float4
fragmentShader(FragmentData in [[stage_in]],
               constant float3 &lightDirection [[buffer(0)]],
               constant float &ambientIntensity [[buffer(1)]],
               constant float4 &color [[buffer(2)]])
{
    if (!((in.modelNDCPosition.x >= -1.0) *
          (in.modelNDCPosition.x <=  1.0) *
          (in.modelNDCPosition.y >= -1.0) *
          (in.modelNDCPosition.y <=  1.0) *
          (in.modelNDCPosition.z >=  0.0) *
          (in.modelNDCPosition.z <=  1.0)))
    {
        discard_fragment();
    }
    
    float diffuseIntensity = saturate(dot(normalize(in.worldNormal), normalize(lightDirection)));
    return float4(saturate(ambientIntensity + diffuseIntensity) * color.rgb, color.a);
}
