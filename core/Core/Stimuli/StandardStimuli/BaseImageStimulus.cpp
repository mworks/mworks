//
//  BaseImageStimulus.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 1/15/21.
//

#include "BaseImageStimulus.hpp"


BEGIN_NAMESPACE_MW


BaseImageStimulus::BaseImageStimulus(const ParameterValueMap &parameters) :
    AlphaBlendedTransformStimulus(parameters),
    texture(nil)
{ }


BaseImageStimulus::~BaseImageStimulus() {
    @autoreleasepool {
        texture = nil;
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
    
    AlphaBlendedTransformStimulus::unloadMetal(display);
}


void BaseImageStimulus::drawMetal(MetalDisplay &display) {
    AlphaBlendedTransformStimulus::drawMetal(display);
    
    auto renderCommandEncoder = createRenderCommandEncoder(display);
    [renderCommandEncoder setRenderPipelineState:renderPipelineState];
    
    float aspectRatio = (fullscreen ? 1.0 : getAspectRatio());
    setVertexBytes(renderCommandEncoder, aspectRatio, 0);
    setCurrentMVPMatrix(display, renderCommandEncoder, 1);
    
    [renderCommandEncoder setFragmentTexture:texture atIndex:0];
    float currentAlpha = current_alpha;
    setFragmentBytes(renderCommandEncoder, currentAlpha, 0);
    
    [renderCommandEncoder drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];
    [renderCommandEncoder endEncoding];
}


END_NAMESPACE_MW
