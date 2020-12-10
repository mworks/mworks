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


void AlphaBlendedTransformStimulus::draw(shared_ptr<StimulusDisplay> display) {
    current_alpha = alpha_multiplier->getValue().getFloat();
    if (!sourceBlendFactorName) {
        current_source_blend_factor = defaultSourceBlendFactor;
    } else {
        current_source_blend_factor_name = sourceBlendFactorName->getValue().getString();
        current_source_blend_factor = blendFactorFromName(current_source_blend_factor_name,
                                                          defaultSourceBlendFactor);
    }
    if (!destBlendFactorName) {
        current_dest_blend_factor = defaultDestBlendFactor;
    } else {
        current_dest_blend_factor_name = destBlendFactorName->getValue().getString();
        current_dest_blend_factor = blendFactorFromName(current_dest_blend_factor_name,
                                                        defaultDestBlendFactor);
    }
    if (!sourceAlphaBlendFactorName) {
        current_source_alpha_blend_factor = current_source_blend_factor;
    } else {
        current_source_alpha_blend_factor_name = sourceAlphaBlendFactorName->getValue().getString();
        current_source_alpha_blend_factor = blendFactorFromName(current_source_alpha_blend_factor_name,
                                                                current_source_blend_factor);
    }
    if (!destAlphaBlendFactorName) {
        current_dest_alpha_blend_factor = current_dest_blend_factor;
    } else {
        current_dest_alpha_blend_factor_name = destAlphaBlendFactorName->getValue().getString();
        current_dest_alpha_blend_factor = blendFactorFromName(current_dest_alpha_blend_factor_name,
                                                              current_dest_blend_factor);
    }
    
    TransformStimulus::draw(display);
    
    last_alpha = current_alpha;
    if (sourceBlendFactorName) {
        last_source_blend_factor_name = current_source_blend_factor_name;
        last_source_blend_factor = current_source_blend_factor;
    }
    if (destBlendFactorName) {
        last_dest_blend_factor_name = current_dest_blend_factor_name;
        last_dest_blend_factor = current_dest_blend_factor;
    }
    if (sourceAlphaBlendFactorName) {
        last_source_alpha_blend_factor_name = current_source_alpha_blend_factor_name;
        last_source_alpha_blend_factor = current_source_alpha_blend_factor;
    }
    if (destAlphaBlendFactorName) {
        last_dest_alpha_blend_factor_name = current_dest_alpha_blend_factor_name;
        last_dest_alpha_blend_factor = current_dest_alpha_blend_factor;
    }
}


Datum AlphaBlendedTransformStimulus::getCurrentAnnounceDrawData() {
    Datum announceData = TransformStimulus::getCurrentAnnounceDrawData();
    
    announceData.addElement(STIM_ALPHA, last_alpha);
    
    // Announce blend factors only if they were set explicitly, in order to avoid
    // adding lots of redundant strings to the event file
    if (sourceBlendFactorName) {
        announceData.addElement(SOURCE_BLEND_FACTOR, last_source_blend_factor_name);
    }
    if (destBlendFactorName) {
        announceData.addElement(DEST_BLEND_FACTOR, last_dest_blend_factor_name);
    }
    if (sourceAlphaBlendFactorName) {
        announceData.addElement(SOURCE_ALPHA_BLEND_FACTOR, last_source_alpha_blend_factor_name);
    }
    if (destAlphaBlendFactorName) {
        announceData.addElement(DEST_ALPHA_BLEND_FACTOR, last_dest_alpha_blend_factor_name);
    }
    
    return announceData;
}


MTLBlendFactor AlphaBlendedTransformStimulus::blendFactorFromName(const std::string &name, MTLBlendFactor fallback) {
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
    merror(M_DISPLAY_MESSAGE_DOMAIN, "Invalid blend factor: %s", name.c_str());
    return fallback;
};


END_NAMESPACE_MW
