//
//  AppleStimulusDisplay.metal
//  MWorksCore
//
//  Created by Christopher Stawarz on 12/11/20.
//

#include <metal_stdlib>

using namespace metal;


struct RasterizerData {
    float4 position [[position]];
    float2 textureCoordinate;
};


vertex RasterizerData
MWKStimulusDisplayViewDelegate_vertexShader(uint vertexID [[vertex_id]])
{
    constexpr float2 vertexPositions[] = {
        { -1.0f, -1.0f },
        {  1.0f, -1.0f },
        { -1.0f,  1.0f },
        {  1.0f,  1.0f }
    };
    
    //
    // All stimuli effectively render using OpenGL texture coordinates ((0,0) at the lower-left corner),
    // which flip y with respect to Metal texture coordinates ((0,0) at the upper-left corner).  Therefore,
    // we similarly y-flip the coordinates we use to draw the framebuffer texture, so that everything
    // ends up oriented correctly on screen.
    //
    constexpr float2 texCoords[] = {
        { 0.0f, 0.0f },
        { 1.0f, 0.0f },
        { 0.0f, 1.0f },
        { 1.0f, 1.0f }
    };
    
    RasterizerData out;
    out.position = float4(vertexPositions[vertexID], 0.0, 1.0);
    out.textureCoordinate = texCoords[vertexID];
    return out;
}


constant bool convertToSRGB [[function_constant(0)]];


float3
linearToSRGB(float3 linear)
{
    linear = saturate(linear);  // Clamp to [0,1]
    float3 srgb = mix(12.92 * linear,
                      1.055 * powr(linear, 1.0/2.4) - 0.055,
                      float3(linear >= 0.0031308));
    // To exactly replicate the linear-to-sRGB conversion performed by the GPU, we must first map the
    // floating-point sRGB color components we just computed back to integers in the range [0, 255],
    // and then re-normalize them to [0.0, 1.0].  Counterintuitively, this rounding process results in
    // better-looking output, with less visible banding in grayscale gradients, than using non-rounded
    // values.
    return round(srgb * 255.0) / 255.0;
}


fragment float4
MWKStimulusDisplayViewDelegate_fragmentShader(RasterizerData in [[stage_in]],
                                              texture2d<float> colorTexture [[texture(0)]])
{
    constexpr sampler textureSampler (filter::linear);
    float4 colorSample = colorTexture.sample(textureSampler, in.textureCoordinate);
    if (convertToSRGB) {
        colorSample.rgb = linearToSRGB(colorSample.rgb);
    }
    return colorSample;
}
