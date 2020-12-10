//
//  MWKMetalView.metal
//  MWorksCore
//
//  Created by Christopher Stawarz on 12/11/20.
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
    { 0.0f, 0.0f },
    { 1.0f, 0.0f },
    { 0.0f, 1.0f },
    { 1.0f, 1.0f },
};


struct RasterizerData {
    float4 clipSpacePosition [[position]];
    float2 textureCoordinate;
};


vertex RasterizerData
MWKMetalView_vertexShader(uint vertexID [[vertex_id]])
{
    RasterizerData out;
    out.clipSpacePosition = float4(vertexPositions[vertexID], 0.0, 1.0);
    out.textureCoordinate = texCoords[vertexID];
    return out;
}


static float3
linearToSRGB(float3 linear)
{
    float3 srgb = mix(12.92 * linear,
                      1.055 * pow(linear, 1.0/2.4) - 0.055,
                      float3(linear >= 0.0031308));
    // To exactly replicate the linear-to-sRGB conversion performed by the GPU, we must first map the
    // floating-point sRGB color components we just computed back to integers in the range [0, 255],
    // and then re-normalize them to [0.0, 1.0].  Counterintuitively, this rounding process results in
    // better-looking output, with less visible banding in grayscale gradients, than using non-rounded
    // values.
    return round(srgb * 255.0) / 255.0;
}


constant bool MWKMetalView_convertToSRGB [[function_constant(0)]];


fragment float4
MWKMetalView_fragmentShader(RasterizerData in [[stage_in]],
                            texture2d<float> colorTexture [[texture(0)]])
{
    constexpr sampler textureSampler (mag_filter::linear, min_filter::linear);
    float4 colorSample = colorTexture.sample(textureSampler, in.textureCoordinate);
    if (MWKMetalView_convertToSRGB) {
        colorSample.rgb = linearToSRGB(colorSample.rgb);
    }
    return colorSample;
}
