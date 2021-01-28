//
//  TransformStimulus.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 12/10/20.
//

#include "TransformStimulus.hpp"

#include "AAPLMathUtilities.h"

using namespace mw::aapl_math_utilities;


BEGIN_NAMESPACE_MW


const std::string TransformStimulus::X_SIZE("x_size");
const std::string TransformStimulus::Y_SIZE("y_size");
const std::string TransformStimulus::X_POSITION("x_position");
const std::string TransformStimulus::Y_POSITION("y_position");
const std::string TransformStimulus::ROTATION("rotation");
const std::string TransformStimulus::FULLSCREEN("fullscreen");


void TransformStimulus::describeComponent(ComponentInfo &info) {
    MetalStimulus::describeComponent(info);
    info.addParameter(X_SIZE, false);
    info.addParameter(Y_SIZE, false);
    info.addParameter(X_POSITION, "0.0");
    info.addParameter(Y_POSITION, "0.0");
    info.addParameter(ROTATION, "0.0");
    info.addParameter(FULLSCREEN, "NO");
}


TransformStimulus::TransformStimulus(const ParameterValueMap &parameters) :
    MetalStimulus(parameters),
    xoffset(registerVariable(parameters[X_POSITION])),
    yoffset(registerVariable(parameters[Y_POSITION])),
    xscale(registerOptionalVariable(optionalVariable(parameters[X_SIZE]))),
    yscale(registerOptionalVariable(optionalVariable(parameters[Y_SIZE]))),
    rotation(registerVariable(parameters[ROTATION])),
    fullscreen(parameters[FULLSCREEN]),
    renderPipelineState(nil)
{
    if (!(xscale || yscale || fullscreen)) {
        throw SimpleException("Either x_size or y_size must be specified");
    }
}


TransformStimulus::~TransformStimulus() {
    @autoreleasepool {
        renderPipelineState = nil;
    }
}


Datum TransformStimulus::getCurrentAnnounceDrawData() {
    auto announceData = MetalStimulus::getCurrentAnnounceDrawData();
    
    if (fullscreen) {
        announceData.addElement(STIM_FULLSCREEN, true);
    } else {
        announceData.addElement(STIM_POSX, current_posx);
        announceData.addElement(STIM_POSY, current_posy);
        announceData.addElement(STIM_SIZEX, current_sizex);
        announceData.addElement(STIM_SIZEY, current_sizey);
        announceData.addElement(STIM_ROT, current_rot);
    }
    
    return announceData;
}


void TransformStimulus::unloadMetal(MetalDisplay &display) {
    renderPipelineState = nil;
}


void TransformStimulus::drawMetal(MetalDisplay &display) {
    if (!fullscreen) {
        current_posx = xoffset->getValue().getFloat();
        current_posy = yoffset->getValue().getFloat();
        getCurrentSize(current_sizex, current_sizey);
        current_rot = rotation->getValue().getFloat();
    }
}


void TransformStimulus::getCurrentSize(float &sizeX, float &sizeY) const {
    if (xscale) {
        sizeX = xscale->getValue().getFloat();
        if (!yscale) {
            sizeY = sizeX;
        }
    }
    if (yscale) {
        sizeY = yscale->getValue().getFloat();
        if (!xscale) {
            sizeX = sizeY;
        }
    }
}


MTLRenderPipelineDescriptor * TransformStimulus::createRenderPipelineDescriptor(MetalDisplay &display,
                                                                                id<MTLFunction> vertexFunction,
                                                                                id<MTLFunction> fragmentFunction) const
{
    auto renderPipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    
    renderPipelineDescriptor.vertexFunction = vertexFunction;
    renderPipelineDescriptor.fragmentFunction = fragmentFunction;
    
    auto colorAttachment = renderPipelineDescriptor.colorAttachments[0];
    colorAttachment.pixelFormat = display.getMetalFramebufferTexturePixelFormat();
    configureBlending(colorAttachment);
    
    return renderPipelineDescriptor;
}


simd::float4x4 TransformStimulus::getCurrentMVPMatrix(const simd::float4x4 &projectionMatrix) const {
    simd::float4x4 currentMVPMatrix;
    
    if (fullscreen) {
        // Scale y by a negative value to match the y flip included in projectionMatrix.  (See
        // AppleStimulusDisplay's constructor for more info.)
        currentMVPMatrix = matrix4x4_scale(2.0, -2.0, 1.0);
    } else {
        currentMVPMatrix = projectionMatrix * matrix4x4_translation(current_posx, current_posy, 0.0);
        currentMVPMatrix = currentMVPMatrix * matrix4x4_rotation(radians_from_degrees(current_rot), 0.0, 0.0, 1.0);
        currentMVPMatrix = currentMVPMatrix * matrix4x4_scale(current_sizex, current_sizey, 1.0);
    }
    
    currentMVPMatrix = currentMVPMatrix * matrix4x4_translation(-0.5, -0.5, 0.0);
    return currentMVPMatrix;
}


END_NAMESPACE_MW
