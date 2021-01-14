//
//  EllipseStimulus.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 1/14/21.
//

#include "EllipseStimulus.hpp"


BEGIN_NAMESPACE_MW


void EllipseStimulus::describeComponent(ComponentInfo &info) {
    ColoredTransformStimulus::describeComponent(info);
    info.setSignature("stimulus/ellipse");
}


Datum EllipseStimulus::getCurrentAnnounceDrawData() {
    auto announceData = ColoredTransformStimulus::getCurrentAnnounceDrawData();
    announceData.addElement(STIM_TYPE, "circle");  // Not "ellipse", for consistency with legacy CircleStimulus
    return announceData;
}


void EllipseStimulus::loadMetal(MetalDisplay &display) {
    ColoredTransformStimulus::loadMetal(display);
    
    auto library = loadDefaultLibrary(display, MWORKS_GET_CURRENT_BUNDLE());
    auto vertexFunction = loadShaderFunction(library, "EllipseStimulus_vertexShader");
    auto fragmentFunction = loadShaderFunction(library, "EllipseStimulus_fragmentShader");
    auto renderPipelineDescriptor = createRenderPipelineDescriptor(display, vertexFunction, fragmentFunction);
    renderPipelineState = createRenderPipelineState(display, renderPipelineDescriptor);
}


void EllipseStimulus::drawMetal(MetalDisplay &display) {
    ColoredTransformStimulus::drawMetal(display);
    
    auto renderCommandEncoder = createRenderCommandEncoder(display);
    setCurrentMVPMatrix(display, renderCommandEncoder, 0);
    setCurrentColor(renderCommandEncoder, 0);
    [renderCommandEncoder drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];
    [renderCommandEncoder endEncoding];
}


END_NAMESPACE_MW
