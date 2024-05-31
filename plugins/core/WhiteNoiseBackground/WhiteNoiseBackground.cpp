/*
 *  WhiteNoiseBackground.cpp
 *  WhiteNoiseBackground
 *
 *  Created by Christopher Stawarz on 12/15/10.
 *  Copyright 2010 MIT. All rights reserved.
 *
 */

#include "WhiteNoiseBackground.h"

#include "WhiteNoiseBackgroundShaderTypes.h"

using namespace mw::white_noise_background;


//
// NOTE: There are some device-dependent optimizations that could improve the performance and/or
// efficiency of this stimulus.  Specifically:
//
//   1. Non-uniform threadgroup size: This would let us use dispatchThreads:threadsPerThreadgroup:
//      (instead of dispatchThreadgroups:threadsPerThreadgroup:) when randomizing the noise and
//      eliminate the need for bounds checking in updateNoise.
//
//   2. Function texture read-writes: This would let us cut the number of textures in half by
//      updating the texture for each channel in place.
//
// These features are available on GPU's that support any of
//
//   MTLGPUFamilyCommon3
//   MTLGPUFamilyApple4
//   MTLGPUFamilyMac1
//
// or, on earlier OS versions that don't support MTLGPUFamily, either of
//
//   MTLFeatureSet_iOS_GPUFamily4_v1
//   MTLFeatureSet_macOS_GPUFamily1_v3
//
// Also, the MTLDevice property readWriteTextureSupport must be at least MTLReadWriteTextureTier1.
//
// However, given the small number of devices (particularly iOS devices) that we currently have for
// testing, it seems unlikely that we could ever fully test a hardware-depedendent implementation
// on all platforms.  Therefore, we opt for the lowest-common-denominator approach, which trades
// any potential performance/efficiency gains for universal hardware support.
//


BEGIN_NAMESPACE_MW


const std::string WhiteNoiseBackground::GRAYSCALE("grayscale");
const std::string WhiteNoiseBackground::GRAIN_SIZE("grain_size");
const std::string WhiteNoiseBackground::RAND_SEED("rand_seed");
const std::string WhiteNoiseBackground::RANDOMIZE_ON_DRAW("randomize_on_draw");


void WhiteNoiseBackground::describeComponent(ComponentInfo &info) {
    Stimulus::describeComponent(info);
    
    info.setSignature("stimulus/white_noise_background");
    
    info.addParameter(GRAYSCALE, "YES");
    info.addParameter(GRAIN_SIZE, "0.0");
    info.addParameter(RAND_SEED, false);
    info.addParameter(RANDOMIZE_ON_DRAW, "NO");
}


WhiteNoiseBackground::WhiteNoiseBackground(const ParameterValueMap &parameters) :
    MetalStimulus(parameters),
    grayscale(parameters[GRAYSCALE]),
    grainSize(parameters[GRAIN_SIZE]),
    randSeed(optionalVariable(parameters[RAND_SEED])),
    randomizeOnDraw(registerVariable(parameters[RANDOMIZE_ON_DRAW])),
    computePipelineState(nil),
    renderPipelineState(nil),
    seedTextures(nil),
    noiseTextures(nil)
{ }


WhiteNoiseBackground::~WhiteNoiseBackground() {
    @autoreleasepool {
        noiseTextures = nil;
        seedTextures = nil;
        renderPipelineState = nil;
        computePipelineState = nil;
    }
}


Datum WhiteNoiseBackground::getCurrentAnnounceDrawData() {
    auto announceData = Stimulus::getCurrentAnnounceDrawData();
    
    announceData.addElement(STIM_TYPE, "white_noise_background");
    announceData.addElement(GRAYSCALE, currentGrayscale);
    announceData.addElement(GRAIN_SIZE, currentGrainSize);
    announceData.addElement(RAND_SEED, currentRandSeed);
    announceData.addElement(RANDOMIZE_ON_DRAW, curentRandomizeOnDraw);
    announceData.addElement("rand_count", static_cast<long long>(randCount));
    
    return announceData;
}


void WhiteNoiseBackground::loadMetal(MetalDisplay &display) {
    //
    // Get required parameter values
    //
    
    currentGrayscale = grayscale->getValue().getBool();
    currentGrainSize = grainSize->getValue().getFloat();
    if (randSeed) {
        currentRandSeed = randSeed->getValue().getInteger();
    } else {
        currentRandSeed = Clock::instance()->getSystemTimeNS();
    }
    
    //
    // Determine texture dimensions
    //
    {
        std::size_t displayWidthPixels, displayHeightPixels;
        display.getCurrentTextureSizeForDisplayArea(displayWidthPixels, displayHeightPixels);
        
        if (currentGrainSize <= 0.0) {
            textureWidth = displayWidthPixels;
            textureHeight = displayHeightPixels;
        } else {
            const auto grainSizePixels = std::max(1.0, display.convertDisplayUnitsToPixels(currentGrainSize));
            textureWidth = std::max(1.0, std::round(double(displayWidthPixels) / grainSizePixels));
            textureHeight = std::max(1.0, std::round(double(displayHeightPixels) / grainSizePixels));
        }
    }
    
    //
    // Create compute and render pipeline states
    //
    {
        auto library = loadDefaultLibrary(display, MWORKS_GET_CURRENT_BUNDLE());
        
        {
            auto function = loadShaderFunction(library, "updateNoise");
            NSError *error = nil;
            computePipelineState = [display.getMetalDevice() newComputePipelineStateWithFunction:function
                                                                                           error:&error];
            if (!computePipelineState) {
                throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN,
                                      "Cannot create Metal compute pipeline state",
                                      error.localizedDescription.UTF8String);
            }
            
            // Calculate compute dispatch parameters, as described at
            // https://developer.apple.com/documentation/metal/calculating_threadgroup_and_grid_sizes
            const auto w = computePipelineState.threadExecutionWidth;
            const auto h = computePipelineState.maxTotalThreadsPerThreadgroup / w;
            threadgroupsPerGrid = MTLSizeMake((textureWidth + w - 1) / w,
                                              (textureHeight + h - 1) / h,
                                              1);
            threadsPerThreadgroup = MTLSizeMake(w, h, 1);
        }
        
        {
            MTLFunctionConstantValues *constantValues = [[MTLFunctionConstantValues alloc] init];
            const bool rgbNoise = !currentGrayscale;
            [constantValues setConstantValue:&rgbNoise
                                        type:MTLDataTypeBool
                                     atIndex:rgbNoiseFunctionConstantIndex];
            
            auto renderPipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
            renderPipelineDescriptor.vertexFunction = loadShaderFunction(library, "vertexShader");
            renderPipelineDescriptor.fragmentFunction = loadShaderFunction(library, "fragmentShader", constantValues);
            renderPipelineDescriptor.colorAttachments[0].pixelFormat = display.getMetalFramebufferTexturePixelFormat();
            renderPipelineState = createRenderPipelineState(display, renderPipelineDescriptor);
        }
    }
    
    //
    // Create textures
    //
    {
        auto textureDescriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatR32Uint
                                                                                    width:textureWidth
                                                                                   height:textureHeight
                                                                                mipmapped:NO];
        textureDescriptor.storageMode = MTLStorageModePrivate;
        textureDescriptor.usage = MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite;
        
        const std::size_t numChannels = (currentGrayscale ? 1 : 3);
        NSMutableArray<id<MTLTexture>> *mutableSeedTextures = [NSMutableArray arrayWithCapacity:numChannels];
        NSMutableArray<id<MTLTexture>> *mutableNoiseTextures = [NSMutableArray arrayWithCapacity:numChannels];
        
        const std::size_t numTexels = textureWidth * textureHeight;
        const std::size_t bufferLength = numTexels * sizeof(MinStdRand::value_type);
        
        // Although we use a linear congruential generator (LCG) on the GPU to update the texels, we use the
        // Mersenne Twister to generate the per-texel seeds.  If we used the same generator in both places,
        // then texel n at iteration i+1 would have the same value as texel n+1 at iteration i.
        std::mt19937 seedGen(currentRandSeed);
        std::uniform_int_distribution<MinStdRand::value_type> seedDist(MinStdRand::min(), MinStdRand::max());
        
        auto commandBuffer = [display.getMetalCommandQueue() commandBuffer];
        auto blitCommandEncoder = [commandBuffer blitCommandEncoder];
        
        do {
            [mutableSeedTextures addObject:[display.getMetalDevice() newTextureWithDescriptor:textureDescriptor]];
            [mutableNoiseTextures addObject:[display.getMetalDevice() newTextureWithDescriptor:textureDescriptor]];
            
            auto buffer = [display.getMetalDevice() newBufferWithLength:bufferLength
                                                                options:MTLResourceStorageModeShared];
            auto bufferData = static_cast<MinStdRand::value_type *>([buffer contents]);
            for (std::size_t i = 0; i < numTexels; i++) {
                bufferData[i] = seedDist(seedGen);
            }
            
            // Only the noise textures need initial data, as they will become the seed textures the
            // first time the noise is randomized
            [blitCommandEncoder copyFromBuffer:buffer
                                  sourceOffset:0
                             sourceBytesPerRow:(bufferLength / textureHeight)
                           sourceBytesPerImage:bufferLength
                                    sourceSize:MTLSizeMake(textureWidth, textureHeight, 1)
                                     toTexture:mutableNoiseTextures.lastObject
                              destinationSlice:0
                              destinationLevel:0
                             destinationOrigin:MTLOriginMake(0, 0, 0)];
        } while (mutableNoiseTextures.count < numChannels);
        
        [blitCommandEncoder endEncoding];
        [commandBuffer commit];
        
        seedTextures = [mutableSeedTextures copy];
        noiseTextures = [mutableNoiseTextures copy];
    }
    
    //
    // Reset randomization-management variables
    //
    
    shouldRandomize = false;
    randCount = 0;
}


void WhiteNoiseBackground::unloadMetal(MetalDisplay &display) {
    noiseTextures = nil;
    seedTextures = nil;
    renderPipelineState = nil;
    computePipelineState = nil;
}


void WhiteNoiseBackground::drawMetal(MetalDisplay &display) {
    //
    // Randomize noise (if needed)
    //
    
    curentRandomizeOnDraw = randomizeOnDraw->getValue().getBool();
    
    if (curentRandomizeOnDraw || shouldRandomize.exchange(false)) {
        // Previous noise values become current seeds
        std::swap(seedTextures, noiseTextures);
        
        auto computeCommandEncoder = [display.getCurrentMetalCommandBuffer() computeCommandEncoder];
        [computeCommandEncoder setComputePipelineState:computePipelineState];
        
        for (NSUInteger channelIndex = 0; channelIndex < seedTextures.count; channelIndex++) {
            [computeCommandEncoder setTexture:seedTextures[channelIndex] atIndex:seedTextureIndex];
            [computeCommandEncoder setTexture:noiseTextures[channelIndex] atIndex:noiseTextureIndex];
            [computeCommandEncoder dispatchThreadgroups:threadgroupsPerGrid
                                  threadsPerThreadgroup:threadsPerThreadgroup];
        }
        
        [computeCommandEncoder endEncoding];
        randCount++;
    }
    
    //
    // Render noise
    //
    
    auto renderCommandEncoder = createRenderCommandEncoder(display);
    [renderCommandEncoder setRenderPipelineState:renderPipelineState];
    
    NSUInteger textureIndex = redNoiseTextureIndex;
    for (id<MTLTexture> texture in noiseTextures) {
        [renderCommandEncoder setFragmentTexture:texture atIndex:textureIndex];
        textureIndex++;
    }
    
    [renderCommandEncoder drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];
    [renderCommandEncoder endEncoding];
}


END_NAMESPACE_MW
