//
//  BaseImageStimulus.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 1/15/21.
//

#include "BaseImageStimulus.hpp"


BEGIN_NAMESPACE_MW


auto BaseImageStimulus::getVertexPositions(double aspectRatio) -> VertexPositionArray {
    if (aspectRatio > 1.0) {
        return makeVertexPositionArray(
            0.0, (0.5 - 0.5/aspectRatio),
            1.0, (0.5 - 0.5/aspectRatio),
            0.0, (0.5 - 0.5/aspectRatio) + 1.0/aspectRatio,
            1.0, (0.5 - 0.5/aspectRatio) + 1.0/aspectRatio
            );
    }
    return makeVertexPositionArray(
        (1.0 - aspectRatio)/2.0,               0.0,
        (1.0 - aspectRatio)/2.0 + aspectRatio, 0.0,
        (1.0 - aspectRatio)/2.0,               1.0,
        (1.0 - aspectRatio)/2.0 + aspectRatio, 1.0
        );
}


BaseImageStimulus::BaseImageStimulus(const ParameterValueMap &parameters) :
    AlphaBlendedTransformStimulus(parameters),
    vertexPositionsBuffer(nil),
    texture(nil)
{ }


BaseImageStimulus::~BaseImageStimulus() {
    @autoreleasepool {
        texture = nil;
        vertexPositionsBuffer = nil;
    }
}


void BaseImageStimulus::loadMetal(MetalDisplay &display) {
    AlphaBlendedTransformStimulus::loadMetal(display);
    
    auto library = loadDefaultLibrary(display, MWORKS_GET_CURRENT_BUNDLE());
    auto vertexFunction = loadShaderFunction(library, "BaseImageStimulus_vertexShader");
    auto fragmentFunction = loadShaderFunction(library, "BaseImageStimulus_fragmentShader");
    auto renderPipelineDescriptor = createRenderPipelineDescriptor(display, vertexFunction, fragmentFunction);
    renderPipelineState = createRenderPipelineState(display, renderPipelineDescriptor);
}


void BaseImageStimulus::unloadMetal(MetalDisplay &display) {
    texture = nil;
    vertexPositionsBuffer = nil;
    
    AlphaBlendedTransformStimulus::unloadMetal(display);
}


void BaseImageStimulus::drawMetal(MetalDisplay &display) {
    AlphaBlendedTransformStimulus::drawMetal(display);
    
    auto renderCommandEncoder = createRenderCommandEncoder(display);
    
    [renderCommandEncoder setVertexBuffer:vertexPositionsBuffer offset:0 atIndex:0];
    setCurrentMVPMatrix(display, renderCommandEncoder, 1);
    
    [renderCommandEncoder setFragmentTexture:texture atIndex:0];
    float currentAlpha = current_alpha;
    [renderCommandEncoder setFragmentBytes:&currentAlpha length:sizeof(currentAlpha) atIndex:0];
    
    [renderCommandEncoder drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];
    [renderCommandEncoder endEncoding];
}


void BaseImageStimulus::setVertexPositions(MetalDisplay &display,
                                           id<MTLBlitCommandEncoder> blitCommandEncoder,
                                           double aspectRatio)
{
    const auto vertexPositions = getVertexPositions(fullscreen ? 1.0 : aspectRatio);
    const auto vertexPositionsSize = sizeof(vertexPositions);
    
    // Create shared buffer
    auto buffer = [display.getMetalDevice() newBufferWithBytes:vertexPositions.data()
                                                        length:vertexPositionsSize
                                                       options:MTLResourceStorageModeShared];
    
    // Create private buffer
    vertexPositionsBuffer = [display.getMetalDevice() newBufferWithLength:vertexPositionsSize
                                                                  options:MTLResourceStorageModePrivate];
    
    // Copy data from shared buffer to private buffer
    [blitCommandEncoder copyFromBuffer:buffer
                          sourceOffset:0
                              toBuffer:vertexPositionsBuffer
                     destinationOffset:0
                                  size:vertexPositionsSize];
}


END_NAMESPACE_MW
