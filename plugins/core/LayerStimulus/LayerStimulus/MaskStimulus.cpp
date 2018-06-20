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


void MaskStimulus::describeComponent(ComponentInfo &info) {
    BasicTransformStimulus::describeComponent(info);
    
    info.setSignature("stimulus/mask");
    
    info.addParameter(MASK);
    info.addParameter(INVERTED, "NO");
    info.addParameter(STD_DEV, "1.0");
    info.addParameter(MEAN, "0.0");
    info.addParameter(NORMALIZED, "NO");
}


MaskStimulus::MaskStimulus(const ParameterValueMap &parameters) :
    BasicTransformStimulus(parameters),
    maskTypeName(registerVariable(variableOrText(parameters[MASK]))),
    inverted(registerVariable(parameters[INVERTED])),
    std_dev(registerVariable(parameters[STD_DEV])),
    mean(registerVariable(parameters[MEAN])),
    normalized(registerVariable(parameters[NORMALIZED]))
{ }


void MaskStimulus::draw(boost::shared_ptr<StimulusDisplay> display) {
    current_mask_type_name = maskTypeName->getValue().getString();
    current_mask_type = maskTypeFromName(current_mask_type_name);
    current_inverted = inverted->getValue().getBool();
    current_std_dev = std_dev->getValue().getFloat();
    current_mean = mean->getValue().getFloat();
    current_normalized = normalized->getValue().getBool();
    
    BasicTransformStimulus::draw(display);
    
    last_mask_type_name = current_mask_type_name;
    last_mask_type = current_mask_type;
    last_inverted = current_inverted;
    last_std_dev = current_std_dev;
    last_mean = current_mean;
    last_normalized = current_normalized;
}


Datum MaskStimulus::getCurrentAnnounceDrawData() {
    auto announceData = BasicTransformStimulus::getCurrentAnnounceDrawData();
    
    announceData.addElement(STIM_TYPE, "mask");
    announceData.addElement(MASK, last_mask_type_name);
    announceData.addElement(INVERTED, last_inverted);
    
    if (last_mask_type == MaskType::gaussian) {
        announceData.addElement(STD_DEV, last_std_dev);
        announceData.addElement(MEAN, last_mean);
        announceData.addElement(NORMALIZED, last_normalized);
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
    }
    merror(M_DISPLAY_MESSAGE_DOMAIN, "Invalid mask type: %s", name.c_str());
    return MaskType::rectangle;
};


gl::Shader MaskStimulus::getVertexShader() const {
    static const std::string source
    (R"(
     uniform mat4 mvpMatrix;
     in vec4 vertexPosition;
     
     out vec2 maskVaryingCoords;
     
     void main() {
         gl_Position = mvpMatrix * vertexPosition;
         maskVaryingCoords = vertexPosition.xy;
     }
     )");
    
    return gl::createShader(GL_VERTEX_SHADER, source);
}


gl::Shader MaskStimulus::getFragmentShader() const {
    static const std::string source
    (R"(
     const int rectangleMask = 1;
     const int ellipseMask = 2;
     const int gaussianMask = 3;
     
     const vec2 maskCenter = vec2(0.5, 0.5);
     const float maskRadius = 0.5;
     const float pi = 3.14159265358979323846264338327950288;
     
     uniform float alpha;
     uniform int maskType;
     uniform bool inverted;
     uniform float stdDev;
     uniform float mean;
     uniform bool normalized;
     
     in vec2 maskVaryingCoords;
     
     out vec4 fragColor;
     
     void main() {
         float maskValue;
         float dist;
         float delta;
         
         switch (maskType) {
             case rectangleMask:
                 maskValue = 1.0;
                 break;
                 
             case ellipseMask:
                 //
                 // For an explanation of the edge-smoothing technique used here, see either of the following:
                 // https://rubendv.be/blog/opengl/drawing-antialiased-circles-in-opengl/
                 // http://www.numb3r23.net/2015/08/17/using-fwidth-for-distance-based-anti-aliasing/
                 //
                 dist = distance(maskVaryingCoords, maskCenter);
                 delta = fwidth(dist);
                 maskValue = 1.0 - smoothstep(maskRadius - delta, maskRadius, dist);
                 break;
                 
             case gaussianMask:
                 dist = distance(maskVaryingCoords, maskCenter) / maskRadius;
                 maskValue = exp(-1.0 * (dist - mean) * (dist - mean) / (2.0 * stdDev * stdDev));
                 if (normalized) {
                     maskValue /= stdDev * sqrt(2.0 * pi);
                 }
                 break;
         }
         
         if (inverted) {
             maskValue = 1.0 - maskValue;
         }
         
         fragColor = vec4(1.0, 1.0, 1.0, alpha * maskValue);
     }
     )");
    
    return gl::createShader(GL_FRAGMENT_SHADER, source);
}


void MaskStimulus::prepare(const boost::shared_ptr<StimulusDisplay> &display) {
    BasicTransformStimulus::prepare(display);
    
    alphaUniformLocation = glGetUniformLocation(program, "alpha");
    maskTypeUniformLocation = glGetUniformLocation(program, "maskType");
    invertedUniformLocation = glGetUniformLocation(program, "inverted");
    stdDevUniformLocation = glGetUniformLocation(program, "stdDev");
    meanUniformLocation = glGetUniformLocation(program, "mean");
    normalizedUniformLocation = glGetUniformLocation(program, "normalized");
}


void MaskStimulus::preDraw(const boost::shared_ptr<StimulusDisplay> &display) {
    BasicTransformStimulus::preDraw(display);
    
    glUniform1f(alphaUniformLocation, current_alpha);
    glUniform1i(maskTypeUniformLocation, int(current_mask_type));
    glUniform1i(invertedUniformLocation, current_inverted);
    glUniform1f(stdDevUniformLocation, current_std_dev);
    glUniform1f(meanUniformLocation, current_mean);
    glUniform1f(normalizedUniformLocation, current_normalized);
    
    // Blend by multiplying the destination color by the mask color
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ZERO, GL_SRC_COLOR);
}


void MaskStimulus::postDraw(const boost::shared_ptr<StimulusDisplay> &display) {
    glDisable(GL_BLEND);
    
    BasicTransformStimulus::postDraw(display);
}


END_NAMESPACE_MW




















