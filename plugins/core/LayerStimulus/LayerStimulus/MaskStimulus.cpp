//
//  MaskStimulus.cpp
//  LayerStimulus
//
//  Created by Christopher Stawarz on 6/19/18.
//  Copyright © 2018 The MWorks Project. All rights reserved.
//

#include "MaskStimulus.hpp"


BEGIN_NAMESPACE_MW


const std::string MaskStimulus::MASK("mask");
const std::string MaskStimulus::INVERTED("inverted");
const std::string MaskStimulus::STD_DEV("std_dev");
const std::string MaskStimulus::MEAN("mean");
const std::string MaskStimulus::NORMALIZED("normalized");
const std::string MaskStimulus::EDGE_WIDTH("edge_width");
const std::string MaskStimulus::COLOR("color");


void MaskStimulus::describeComponent(ComponentInfo &info) {
    TransformStimulus::describeComponent(info);
    
    info.setSignature("stimulus/mask");
    
    info.addParameter(MASK);
    info.addParameter(INVERTED, "NO");
    info.addParameter(STD_DEV, "1.0");
    info.addParameter(MEAN, "0.0");
    info.addParameter(NORMALIZED, "NO");
    info.addParameter(EDGE_WIDTH, "0.125");
    info.addParameter(COLOR, false);
}


MaskStimulus::MaskStimulus(const ParameterValueMap &parameters) :
    TransformStimulus(parameters),
    maskTypeName(registerVariable(variableOrText(parameters[MASK]))),
    inverted(registerVariable(parameters[INVERTED])),
    std_dev(registerVariable(parameters[STD_DEV])),
    mean(registerVariable(parameters[MEAN])),
    normalized(registerVariable(parameters[NORMALIZED])),
    edgeWidth(registerVariable(parameters[EDGE_WIDTH])),
    haveColor(!parameters[COLOR].empty())
{
    if (haveColor) {
        ParsedColorTrio pct(parameters[COLOR]);
        colorRed = registerVariable(pct.getR());
        colorGreen = registerVariable(pct.getG());
        colorBlue = registerVariable(pct.getB());
    }
}


Datum MaskStimulus::getCurrentAnnounceDrawData() {
    auto announceData = TransformStimulus::getCurrentAnnounceDrawData();
    
    announceData.addElement(STIM_TYPE, "mask");
    
    announceData.addElement(MASK, current_mask_type_name);
    announceData.addElement(INVERTED, current_inverted);
    
    switch (current_mask_type) {
        case MaskType::gaussian:
            announceData.addElement(STD_DEV, current_std_dev);
            announceData.addElement(MEAN, current_mean);
            announceData.addElement(NORMALIZED, current_normalized);
            break;
            
        case MaskType::raisedCosine:
            announceData.addElement(EDGE_WIDTH, current_edge_width);
            break;
            
        default:
            break;
    }
    
    if (haveColor) {
        announceData.addElement(STIM_COLOR_R, current_color_red);
        announceData.addElement(STIM_COLOR_G, current_color_green);
        announceData.addElement(STIM_COLOR_B, current_color_blue);
    }
    
    return announceData;
}


auto MaskStimulus::maskTypeFromName(const std::string &name) -> MaskType {
    if (name == "rectangle") {
        return MaskType::rectangle;
    } else if (name == "ellipse") {
        return MaskType::ellipse;
    } else if (name == "gaussian") {
        return MaskType::gaussian;
    } else if (name == "raised_cosine") {
        return MaskType::raisedCosine;
    }
    merror(M_DISPLAY_MESSAGE_DOMAIN, "Invalid mask type: %s", name.c_str());
    return MaskType::rectangle;
};


void MaskStimulus::loadMetal(MetalDisplay &display) {
    TransformStimulus::loadMetal(display);
    
    MTLFunctionConstantValues *constantValues = [[MTLFunctionConstantValues alloc] init];
    [constantValues setConstantValue:&haveColor type:MTLDataTypeBool atIndex:0];
    
    auto library = loadDefaultLibrary(display, MWORKS_GET_CURRENT_BUNDLE());
    auto vertexFunction = loadShaderFunction(library, "MaskStimulus_vertexShader");
    auto fragmentFunction = loadShaderFunction(library, "MaskStimulus_fragmentShader", constantValues);
    auto renderPipelineDescriptor = createRenderPipelineDescriptor(display, vertexFunction, fragmentFunction);
    renderPipelineState = createRenderPipelineState(display, renderPipelineDescriptor);
}


void MaskStimulus::drawMetal(MetalDisplay &display) {
    TransformStimulus::drawMetal(display);
    
    current_mask_type_name = maskTypeName->getValue().getString();
    current_mask_type = maskTypeFromName(current_mask_type_name);
    current_inverted = inverted->getValue().getBool();
    current_std_dev = std_dev->getValue().getFloat();
    current_mean = mean->getValue().getFloat();
    current_normalized = normalized->getValue().getBool();
    current_edge_width = edgeWidth->getValue().getFloat();
    
    if (haveColor) {
        current_color_red = colorRed->getValue().getFloat();
        current_color_green = colorGreen->getValue().getFloat();
        current_color_blue = colorBlue->getValue().getFloat();
    }
    
    auto renderCommandEncoder = createRenderCommandEncoder(display);
    [renderCommandEncoder setRenderPipelineState:renderPipelineState];
    
    setCurrentMVPMatrix(display, renderCommandEncoder, 0);
    
    MaskParameters maskParams;
    maskParams.maskType = current_mask_type;
    maskParams.inverted = current_inverted;
    maskParams.stdDev = current_std_dev;
    maskParams.mean = current_mean;
    maskParams.normalized = current_normalized;
    maskParams.edgeWidth = current_edge_width;
    setFragmentBytes(renderCommandEncoder, maskParams, 0);
    
    if (haveColor) {
        auto currentColor = simd::make_float3(current_color_red, current_color_green, current_color_blue);
        setFragmentBytes(renderCommandEncoder, currentColor, 1);
    }
    
    [renderCommandEncoder drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];
    [renderCommandEncoder endEncoding];
}


void MaskStimulus::configureBlending(MTLRenderPipelineColorAttachmentDescriptor *colorAttachment) const {
    colorAttachment.blendingEnabled = YES;
    
    if (haveColor) {
        // One-minus-alpha blend the mask RGB with the destination RGB
        colorAttachment.sourceRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
        colorAttachment.destinationRGBBlendFactor = MTLBlendFactorSourceAlpha;
        
        // Leave the destination alpha unchanged
        colorAttachment.sourceAlphaBlendFactor = MTLBlendFactorZero;
        colorAttachment.destinationAlphaBlendFactor = MTLBlendFactorOne;
    } else {
        // Leave the destination RGB unchanged
        colorAttachment.sourceRGBBlendFactor = MTLBlendFactorZero;
        colorAttachment.destinationRGBBlendFactor = MTLBlendFactorOne;
        
        // Multiply the destination alpha by the mask alpha
        colorAttachment.sourceAlphaBlendFactor = MTLBlendFactorZero;
        colorAttachment.destinationAlphaBlendFactor = MTLBlendFactorSourceAlpha;
    }
}


END_NAMESPACE_MW
