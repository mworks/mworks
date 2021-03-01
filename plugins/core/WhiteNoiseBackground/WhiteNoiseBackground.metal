//
//  WhiteNoiseBackground.metal
//  WhiteNoiseBackground
//
//  Created by Christopher Stawarz on 3/4/21.
//

#include <metal_stdlib>

using namespace metal;

#include "WhiteNoiseBackgroundShaderTypes.h"


//
// The technique we use to generate noise on the GPU is adapted from Chapter 37, "Efficient Random Number
// Generation and Application Using CUDA", in "GPU Gems 3":
//
// https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch37.html
//


kernel void
updateNoise(texture2d<uint, access::read> seedTexture [[texture(seedTextureIndex)]],
            texture2d<uint, access::write> noiseTexture [[texture(noiseTextureIndex)]],
            uint2 pos [[thread_position_in_grid]])
{
    if (pos.x >= seedTexture.get_width() || pos.y >= seedTexture.get_height()) {
        return;
    }
    uint seed = seedTexture.read(pos).r;
    uint noise = MinStdRand::next(seed);
    noiseTexture.write(uint4(noise, 0, 0, 0), pos);
}


struct RasterizerData {
    float4 position [[position]];
    float2 textureCoordinate;
};


vertex RasterizerData
vertexShader(uint vertexID [[vertex_id]])
{
    constexpr float2 vertexPositions[] = {
        { -1.0f, -1.0f },
        {  1.0f, -1.0f },
        { -1.0f,  1.0f },
        {  1.0f,  1.0f }
    };
    
    constexpr float2 texCoords[] = {
        { 0.0f, 1.0f },
        { 1.0f, 1.0f },
        { 0.0f, 0.0f },
        { 1.0f, 0.0f }
    };
    
    RasterizerData out;
    out.position = float4(vertexPositions[vertexID], 0.0, 1.0);
    out.textureCoordinate = texCoords[vertexID];
    return out;
}


constant bool rgbNoise [[function_constant(rgbNoiseFunctionConstantIndex)]];


float
getNoise(texture2d<uint> noiseTexture,
         float2 textureCoordinate)
{
    constexpr sampler textureSampler (filter::nearest);
    return MinStdRand::normalize(noiseTexture.sample(textureSampler, textureCoordinate).r);
}


fragment float4
fragmentShader(RasterizerData in [[stage_in]],
               texture2d<uint> redNoiseTexture [[texture(redNoiseTextureIndex)]],
               texture2d<uint> greenNoiseTexture [[texture(greenNoiseTextureIndex), function_constant(rgbNoise)]],
               texture2d<uint> blueNoiseTexture [[texture(blueNoiseTextureIndex), function_constant(rgbNoise)]])
{
    float4 colorSample = float4(0.0, 0.0, 0.0, 1.0);
    float redNoise = getNoise(redNoiseTexture, in.textureCoordinate);
    if (!rgbNoise) {
        colorSample.rgb = float3(redNoise);
    } else {
        float greenNoise = getNoise(greenNoiseTexture, in.textureCoordinate);
        float blueNoise = getNoise(blueNoiseTexture, in.textureCoordinate);
        colorSample.rgb = float3(redNoise, greenNoise, blueNoise);
    }
    return colorSample;
}
