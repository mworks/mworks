//
//  AlphaBlendedTransformStimulus.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 12/10/20.
//

#include "AlphaBlendedTransformStimulus.hpp"


BEGIN_NAMESPACE_MW


const std::string AlphaBlendedTransformStimulus::ALPHA_MULTIPLIER("alpha_multiplier");
const std::string AlphaBlendedTransformStimulus::SOURCE_BLEND_FACTOR("source_blend_factor");
const std::string AlphaBlendedTransformStimulus::DEST_BLEND_FACTOR("dest_blend_factor");
const std::string AlphaBlendedTransformStimulus::SOURCE_ALPHA_BLEND_FACTOR("source_alpha_blend_factor");
const std::string AlphaBlendedTransformStimulus::DEST_ALPHA_BLEND_FACTOR("dest_alpha_blend_factor");


void AlphaBlendedTransformStimulus::describeComponent(ComponentInfo &info) {
    TransformStimulus::describeComponent(info);
    info.addParameter(ALPHA_MULTIPLIER, "1.0");
    info.addParameter(SOURCE_BLEND_FACTOR, false);
    info.addParameter(DEST_BLEND_FACTOR, false);
    info.addParameter(SOURCE_ALPHA_BLEND_FACTOR, false);
    info.addParameter(DEST_ALPHA_BLEND_FACTOR, false);
}


AlphaBlendedTransformStimulus::AlphaBlendedTransformStimulus(const ParameterValueMap &parameters) :
    TransformStimulus(parameters),
    alpha_multiplier(registerVariable(parameters[ALPHA_MULTIPLIER])),
    sourceBlendFactorName(registerOptionalVariable(optionalVariableOrText(parameters[SOURCE_BLEND_FACTOR]))),
    destBlendFactorName(registerOptionalVariable(optionalVariableOrText(parameters[DEST_BLEND_FACTOR]))),
    sourceAlphaBlendFactorName(registerOptionalVariable(optionalVariableOrText(parameters[SOURCE_ALPHA_BLEND_FACTOR]))),
    destAlphaBlendFactorName(registerOptionalVariable(optionalVariableOrText(parameters[DEST_ALPHA_BLEND_FACTOR])))
{ }


Datum AlphaBlendedTransformStimulus::getCurrentAnnounceDrawData() {
    auto announceData = TransformStimulus::getCurrentAnnounceDrawData();
    
    announceData.addElement(STIM_ALPHA, current_alpha);
    
    // Announce blend factors only if they were set explicitly, in order to avoid
    // adding lots of redundant strings to the event file
    if (sourceBlendFactorName) {
        announceData.addElement(SOURCE_BLEND_FACTOR, current_source_blend_factor_name);
    }
    if (destBlendFactorName) {
        announceData.addElement(DEST_BLEND_FACTOR, current_dest_blend_factor_name);
    }
    if (sourceAlphaBlendFactorName) {
        announceData.addElement(SOURCE_ALPHA_BLEND_FACTOR, current_source_alpha_blend_factor_name);
    }
    if (destAlphaBlendFactorName) {
        announceData.addElement(DEST_ALPHA_BLEND_FACTOR, current_dest_alpha_blend_factor_name);
    }
    
    return announceData;
}


MTLBlendFactor AlphaBlendedTransformStimulus::blendFactorFromName(const std::string &name) {
    if (name == "zero") {
        return MTLBlendFactorZero;
    } else if (name == "one") {
        return MTLBlendFactorOne;
    } else if (name == "source_color") {
        return MTLBlendFactorSourceColor;
    } else if (name == "one_minus_source_color") {
        return MTLBlendFactorOneMinusSourceColor;
    } else if (name == "dest_color") {
        return MTLBlendFactorDestinationColor;
    } else if (name == "one_minus_dest_color") {
        return MTLBlendFactorOneMinusDestinationColor;
    } else if (name == "source_alpha") {
        return MTLBlendFactorSourceAlpha;
    } else if (name == "one_minus_source_alpha") {
        return MTLBlendFactorOneMinusSourceAlpha;
    } else if (name == "dest_alpha") {
        return MTLBlendFactorDestinationAlpha;
    } else if (name == "one_minus_dest_alpha") {
        return MTLBlendFactorOneMinusDestinationAlpha;
    }
    throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Invalid blend factor", name);
};


void AlphaBlendedTransformStimulus::loadMetal(MetalDisplay &display) {
    TransformStimulus::loadMetal(display);
    
    if (!sourceBlendFactorName) {
        current_source_blend_factor = MTLBlendFactorSourceAlpha;
    } else {
        current_source_blend_factor_name = sourceBlendFactorName->getValue().getString();
        current_source_blend_factor = blendFactorFromName(current_source_blend_factor_name);
    }
    if (!destBlendFactorName) {
        current_dest_blend_factor = MTLBlendFactorOneMinusSourceAlpha;
    } else {
        current_dest_blend_factor_name = destBlendFactorName->getValue().getString();
        current_dest_blend_factor = blendFactorFromName(current_dest_blend_factor_name);
    }
    if (!sourceAlphaBlendFactorName) {
        current_source_alpha_blend_factor = current_source_blend_factor;
    } else {
        current_source_alpha_blend_factor_name = sourceAlphaBlendFactorName->getValue().getString();
        current_source_alpha_blend_factor = blendFactorFromName(current_source_alpha_blend_factor_name);
    }
    if (!destAlphaBlendFactorName) {
        current_dest_alpha_blend_factor = current_dest_blend_factor;
    } else {
        current_dest_alpha_blend_factor_name = destAlphaBlendFactorName->getValue().getString();
        current_dest_alpha_blend_factor = blendFactorFromName(current_dest_alpha_blend_factor_name);
    }
}


void AlphaBlendedTransformStimulus::drawMetal(MetalDisplay &display) {
    TransformStimulus::drawMetal(display);
    
    current_alpha = alpha_multiplier->getValue().getFloat();
}


MTLRenderPipelineDescriptor *
AlphaBlendedTransformStimulus::createRenderPipelineDescriptor(MetalDisplay &display,
                                                              id<MTLFunction> vertexFunction,
                                                              id<MTLFunction> fragmentFunction) const
{
    auto renderPipelineDescriptor = TransformStimulus::createRenderPipelineDescriptor(display,
                                                                                      vertexFunction,
                                                                                      fragmentFunction);
    
    auto colorAttachment = renderPipelineDescriptor.colorAttachments[0];
    colorAttachment.blendingEnabled = YES;
    colorAttachment.sourceRGBBlendFactor = current_source_blend_factor;
    colorAttachment.destinationRGBBlendFactor = current_dest_blend_factor;
    colorAttachment.sourceAlphaBlendFactor = current_source_alpha_blend_factor;
    colorAttachment.destinationAlphaBlendFactor = current_dest_alpha_blend_factor;
    
    return renderPipelineDescriptor;
}


END_NAMESPACE_MW
