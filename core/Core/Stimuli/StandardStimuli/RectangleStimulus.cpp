//
//  RectangleStimulus.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 12/10/20.
//

#include "RectangleStimulus.hpp"


BEGIN_NAMESPACE_MW


void RectangleStimulus::describeComponent(ComponentInfo &info) {
    ColoredTransformStimulus::describeComponent(info);
    info.setSignature("stimulus/rectangle");
}


Datum RectangleStimulus::getCurrentAnnounceDrawData() {
    auto announceData = ColoredTransformStimulus::getCurrentAnnounceDrawData();
    announceData.addElement(STIM_TYPE, "rectangle");
    return announceData;
}


void RectangleStimulus::loadMetal(MetalDisplay &display) {
    ColoredTransformStimulus::loadMetal(display);
    
    auto library = loadDefaultLibrary(display, MWORKS_GET_CURRENT_BUNDLE());
    auto vertexFunction = loadShaderFunction(library, "RectangleStimulus_vertexShader");
    auto fragmentFunction = loadShaderFunction(library, "RectangleStimulus_fragmentShader");
    auto renderPipelineDescriptor = createRenderPipelineDescriptor(display, vertexFunction, fragmentFunction);
    renderPipelineState = createRenderPipelineState(display, renderPipelineDescriptor);
}


void RectangleStimulus::drawMetal(MetalDisplay &display) {
    ColoredTransformStimulus::drawMetal(display);
    
    auto renderCommandEncoder = createRenderCommandEncoder(display);
    [renderCommandEncoder setRenderPipelineState:renderPipelineState];
    setCurrentMVPMatrix(display, renderCommandEncoder, 0);
    setCurrentColor(renderCommandEncoder, 0);
    [renderCommandEncoder drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];
    [renderCommandEncoder endEncoding];
}


END_NAMESPACE_MW
