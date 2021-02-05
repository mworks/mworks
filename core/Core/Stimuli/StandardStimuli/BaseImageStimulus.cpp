//
//  BaseImageStimulus.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 1/15/21.
//

#include "BaseImageStimulus.hpp"


BEGIN_NAMESPACE_MW


void BaseImageStimulus::loadMetal(MetalDisplay &display) {
    AlphaBlendedTransformStimulus::loadMetal(display);
    
    auto library = loadDefaultLibrary(display, MWORKS_GET_CURRENT_BUNDLE());
    auto vertexFunction = loadShaderFunction(library, "BaseImageStimulus_vertexShader");
    auto fragmentFunction = loadShaderFunction(library, "BaseImageStimulus_fragmentShader");
    auto renderPipelineDescriptor = createRenderPipelineDescriptor(display, vertexFunction, fragmentFunction);
    renderPipelineState = createRenderPipelineState(display, renderPipelineDescriptor);
}


void BaseImageStimulus::drawMetal(MetalDisplay &display) {
    AlphaBlendedTransformStimulus::drawMetal(display);
    
    if (!prepareCurrentTexture(display)) {
        return;
    }
    
    auto renderCommandEncoder = createRenderCommandEncoder(display);
    [renderCommandEncoder setRenderPipelineState:renderPipelineState];
    
    float currentAspectRatio = (fullscreen ? 1.0 : getCurrentAspectRatio());
    setVertexBytes(renderCommandEncoder, currentAspectRatio, 0);
    setCurrentMVPMatrix(display, renderCommandEncoder, 1);
    
    [renderCommandEncoder setFragmentTexture:getCurrentTexture() atIndex:0];
    float currentAlpha = current_alpha;
    setFragmentBytes(renderCommandEncoder, currentAlpha, 0);
    
    [renderCommandEncoder drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];
    [renderCommandEncoder endEncoding];
}


END_NAMESPACE_MW
