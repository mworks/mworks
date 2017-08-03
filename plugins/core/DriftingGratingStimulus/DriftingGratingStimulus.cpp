/*
 *  DriftingGratingStimulus.cpp
 *  MWorksCore
 *
 *  Created by bkennedy on 8/26/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#include "DriftingGratingStimulus.h"


BEGIN_NAMESPACE_MW


const std::string DriftingGratingStimulus::DIRECTION("direction");
const std::string DriftingGratingStimulus::STARTING_PHASE("starting_phase");
const std::string DriftingGratingStimulus::FREQUENCY("spatial_frequency");
const std::string DriftingGratingStimulus::SPEED("speed");
const std::string DriftingGratingStimulus::GRATING_TYPE("grating_type");
const std::string DriftingGratingStimulus::MASK("mask");
const std::string DriftingGratingStimulus::INVERTED("inverted");
const std::string DriftingGratingStimulus::STD_DEV("std_dev");
const std::string DriftingGratingStimulus::MEAN("mean");
const std::string DriftingGratingStimulus::NORMALIZED("normalized");


void DriftingGratingStimulus::describeComponent(ComponentInfo &info) {
    DriftingGratingStimulusBase::describeComponent(info);
    
    info.setSignature("stimulus/drifting_grating");
    
    info.addParameter(DIRECTION, "0.0");
    info.addParameter(STARTING_PHASE, "0.0");
    info.addParameter(FREQUENCY);
    info.addParameter(SPEED);
    info.addParameter(GRATING_TYPE);
    info.addParameter(MASK);
    info.addParameter(INVERTED, "NO");
    info.addParameter(STD_DEV, "1.0");
    info.addParameter(MEAN, "0.0");
    info.addParameter(NORMALIZED, "YES");
    
    // This is no longer used but may be present in old experiments
    info.addIgnoredParameter("grating_sample_rate");
}


auto DriftingGratingStimulus::gratingTypeFromName(const std::string &name) -> GratingType {
    if (name == "sinusoid") {
        return GratingType::sinusoid;
    } else if (name == "square") {
        return GratingType::square;
    } else if (name == "triangle") {
        return GratingType::triangle;
    } else if (name == "sawtooth") {
        return GratingType::sawtooth;
    }
    throw SimpleException("Invalid grating type", name);
};


auto DriftingGratingStimulus::maskTypeFromName(const std::string &name) -> MaskType {
    if (name == "rectangle") {
        return MaskType::rectangle;
    } else if (name == "ellipse") {
        return MaskType::ellipse;
    } else if (name == "gaussian") {
        return MaskType::gaussian;
    }
    throw SimpleException("Invalid mask type", name);
};


DriftingGratingStimulus::DriftingGratingStimulus(const ParameterValueMap &parameters) :
    DriftingGratingStimulusBase(parameters),
    direction_in_degrees(registerVariable(parameters[DIRECTION])),
    starting_phase(registerVariable(parameters[STARTING_PHASE])),
    spatial_frequency(registerVariable(parameters[FREQUENCY])),
    speed(registerVariable(parameters[SPEED])),
    gratingTypeName(parameters[GRATING_TYPE].str()),
    gratingType(gratingTypeFromName(gratingTypeName)),
    maskTypeName(parameters[MASK].str()),
    maskType(maskTypeFromName(maskTypeName)),
    inverted(registerVariable(parameters[INVERTED])),
    std_dev(registerVariable(parameters[STD_DEV])),
    mean(registerVariable(parameters[MEAN])),
    normalized(registerVariable(parameters[NORMALIZED]))
{ }


Datum DriftingGratingStimulus::getCurrentAnnounceDrawData() {
    boost::mutex::scoped_lock locker(stim_lock);
    
    Datum announceData = DriftingGratingStimulusBase::getCurrentAnnounceDrawData();
    
    announceData.addElement(STIM_TYPE, "drifting_grating");
    announceData.addElement(DIRECTION, last_direction_in_degrees);
    announceData.addElement(STARTING_PHASE, last_starting_phase);
    announceData.addElement("current_phase", last_phase);
    announceData.addElement(FREQUENCY, last_spatial_frequency);
    announceData.addElement(SPEED, last_speed);
    announceData.addElement(GRATING_TYPE, gratingTypeName);
    announceData.addElement(MASK, maskTypeName);
    
    if (gratingType == GratingType::sawtooth) {
        announceData.addElement(INVERTED, last_inverted);
    }
    
    if (maskType == MaskType::gaussian) {
        announceData.addElement(STD_DEV, last_std_dev);
        announceData.addElement(MEAN, last_mean);
        announceData.addElement(NORMALIZED, last_normalized);
    }
    
    return announceData;
}


gl::Shader DriftingGratingStimulus::getVertexShader() const {
    static const std::string source
    (R"(
     uniform mat4 mvpMatrix;
     
     in vec4 vertexPosition;
     in float gratingCoord;
     in vec2 maskCoords;
     
     out float gratingVaryingCoord;
     out vec2 maskVaryingCoords;
     
     void main() {
         gl_Position = mvpMatrix * vertexPosition;
         gratingVaryingCoord = gratingCoord;
         maskVaryingCoords = maskCoords;
     }
     )");
    
    return gl::createShader(GL_VERTEX_SHADER, source);
}


gl::Shader DriftingGratingStimulus::getFragmentShader() const {
    static const std::string source
    (R"(
     const int sinusoidGrating = 1;
     const int squareGrating = 2;
     const int triangleGrating = 3;
     const int sawtoothGrating = 4;
     
     const int rectangleMask = 1;
     const int ellipseMask = 2;
     const int gaussianMask = 3;
     
     const vec2 maskCenter = vec2(0.5, 0.5);
     const float maskRadius = 0.5;
     const float pi = 3.14159265358979323846264338327950288;
     
     uniform int gratingType;
     uniform int maskType;
     uniform vec4 color;
     uniform bool inverted;
     uniform float stdDev;
     uniform float mean;
     uniform bool normalized;
     
     in float gratingVaryingCoord;
     in vec2 maskVaryingCoords;
     
     out vec4 fragColor;
     
     void main() {
         if (maskVaryingCoords.x < 0.0 || maskVaryingCoords.x > 1.0 ||
             maskVaryingCoords.y < 0.0 || maskVaryingCoords.y > 1.0)
         {
             discard;
         }
         
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
                 if (dist > maskRadius) {
                     discard;
                 }
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
         
         float normGratingCoord = mod(gratingVaryingCoord, 1.0);
         float gratingValue;
         switch (gratingType) {
             case sinusoidGrating:
                 gratingValue = 0.5*(1.0+cos(2.0*pi*normGratingCoord));
                 break;
                 
             case squareGrating:
                 if (cos(2.0*pi*normGratingCoord) > 0.0) {
                     gratingValue = 1.0;
                 } else {
                     gratingValue = 0.0;
                 }
                 break;
                 
             case triangleGrating:
                 if (normGratingCoord < 0.5) {
                     gratingValue = (0.5 - normGratingCoord) / 0.5;
                 } else {
                     gratingValue = (normGratingCoord - 0.5) / 0.5;
                 }
                 break;
                 
             case sawtoothGrating:
                 if (inverted) {
                     gratingValue = 1.0 - normGratingCoord;
                 } else {
                     gratingValue = normGratingCoord;
                 }
                 break;
         }
         
         fragColor.rgb = gratingValue * color.rgb;
         fragColor.a = color.a * maskValue;
     }
     )");
    
    return gl::createShader(GL_FRAGMENT_SHADER, source);
}


auto DriftingGratingStimulus::getVertexPositions() const -> VertexPositionArray {
    return VertexPositionArray {
        -0.5f, -0.5f,
         0.5f, -0.5f,
        -0.5f,  0.5f,
         0.5f,  0.5f
    };
}


GLKMatrix4 DriftingGratingStimulus::getCurrentMVPMatrix(const GLKMatrix4 &projectionMatrix) const {
    auto currentMVPMatrix = GLKMatrix4Translate(projectionMatrix, current_posx, current_posy, 0.0);
    currentMVPMatrix = GLKMatrix4Rotate(currentMVPMatrix, GLKMathDegreesToRadians(current_rot), 0.0, 0.0, 1.0);
    auto scale_size = std::max(current_sizex, current_sizey);
    return GLKMatrix4Scale(currentMVPMatrix, scale_size, scale_size, 1.0);
}


void DriftingGratingStimulus::prepare(const boost::shared_ptr<StimulusDisplay> &display) {
    boost::mutex::scoped_lock locker(stim_lock);
    
    ColoredTransformStimulus::prepare(display);
    
    glUniform1i(glGetUniformLocation(program, "gratingType"), int(gratingType));
    glUniform1i(glGetUniformLocation(program, "maskType"), int(maskType));
    
    invertedUniformLocation = glGetUniformLocation(program, "inverted");
    stdDevUniformLocation = glGetUniformLocation(program, "stdDev");
    meanUniformLocation = glGetUniformLocation(program, "mean");
    normalizedUniformLocation = glGetUniformLocation(program, "normalized");
    
    glGenBuffers(1, &gratingCoordBuffer);
    gl::BufferBinding<GL_ARRAY_BUFFER> arrayBufferBinding(gratingCoordBuffer);
    GLint gratingCoordAttribLocation = glGetAttribLocation(program, "gratingCoord");
    glEnableVertexAttribArray(gratingCoordAttribLocation);
    glVertexAttribPointer(gratingCoordAttribLocation, 1, GL_FLOAT, GL_FALSE, 0, nullptr);
    
    glGenBuffers(1, &maskCoordsBuffer);
    arrayBufferBinding.bind(maskCoordsBuffer);
    GLint maskCoordsAttribLocation = glGetAttribLocation(program, "maskCoords");
    glEnableVertexAttribArray(maskCoordsAttribLocation);
    glVertexAttribPointer(maskCoordsAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
}


void DriftingGratingStimulus::destroy(const boost::shared_ptr<StimulusDisplay> &display) {
    boost::mutex::scoped_lock locker(stim_lock);
    
    glDeleteBuffers(1, &maskCoordsBuffer);
    glDeleteBuffers(1, &gratingCoordBuffer);
    
    ColoredTransformStimulus::destroy(display);
}


void DriftingGratingStimulus::preDraw(const boost::shared_ptr<StimulusDisplay> &display) {
    ColoredTransformStimulus::preDraw(display);
    
    glUniform1i(invertedUniformLocation, current_inverted);
    glUniform1f(stdDevUniformLocation, current_std_dev);
    glUniform1f(meanUniformLocation, current_mean);
    glUniform1f(normalizedUniformLocation, current_normalized);
    
    // here's the description of this equation
    // starting_phase is in degrees ->  degrees*pi/180 = radians
    //
    // speed is in degrees/second
    // spatial_frequency is in cycles/degree
    // elapsed_time is time since the start the 'play' in us
    // us/100000 = seconds
    //
    // degrees   cycles    1 second    pi radians
    // ------- * ------ * ---------- * ----------- * us = radians of phase
    //    s      degree   1000000 us   180 degrees
    //
    // multiply by -1 so it the grating goes in the correct direction
    MWTime elapsed_time = getElapsedTime();
    double elapsed_seconds = (double)elapsed_time /  (double)1000000;
    const double phase = -1*(current_starting_phase*(M_PI/180.) + current_speed*current_spatial_frequency*(2.*M_PI)*elapsed_seconds);
    const double direction_in_radians = current_direction_in_degrees*(M_PI/180.);
    
    const float f = std::cos(direction_in_radians);
    const float g = std::sin(direction_in_radians);
    const float d = ((f+g)-1)/2;
    const float grating_bl = 0-d;
    const float grating_br = grating_bl+f;
    const float grating_tr = 1+d;
    const float grating_tl = grating_bl+g;
    
    const float phase_proportion = phase/(2*M_PI);
    const float cycle_proportion = current_spatial_frequency * std::max(current_sizex, current_sizey);
    
    std::array<GLfloat, 4 * 1> gratingCoord = {
        (cycle_proportion * grating_bl) + phase_proportion,
        (cycle_proportion * grating_br) + phase_proportion,
        (cycle_proportion * grating_tl) + phase_proportion,
        (cycle_proportion * grating_tr) + phase_proportion
    };
    gl::BufferBinding<GL_ARRAY_BUFFER> arrayBufferBinding(gratingCoordBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gratingCoord), gratingCoord.data(), GL_STREAM_DRAW);
    
    const double aspect = current_sizex / current_sizey;
    float mask_x_offset = 0;
    float mask_y_offset = 0;
    if (aspect > 1.0) {
        mask_y_offset = (aspect - 1.0) / 2.0;
    } else if (aspect < 1.0) {
        mask_x_offset = (1.0/aspect - 1.0) / 2.0;
    }
    
    std::array<GLfloat, 4 * 2> maskCoords = {
        0 - mask_x_offset, 0 - mask_y_offset,
        1 + mask_x_offset, 0 - mask_y_offset,
        0 - mask_x_offset, 1 + mask_y_offset,
        1 + mask_x_offset, 1 + mask_y_offset
    };
    arrayBufferBinding.bind(maskCoordsBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(maskCoords), maskCoords.data(), GL_STREAM_DRAW);
    
    last_phase = phase*(180/M_PI);
}


void DriftingGratingStimulus::drawFrame(boost::shared_ptr<StimulusDisplay> display) {
    current_direction_in_degrees = direction_in_degrees->getValue().getFloat();
    current_starting_phase = starting_phase->getValue().getFloat();
    current_spatial_frequency = spatial_frequency->getValue().getFloat();
    current_speed = speed->getValue().getFloat();
    current_inverted = inverted->getValue().getBool();
    current_std_dev = std_dev->getValue().getFloat();
    current_mean = mean->getValue().getFloat();
    current_normalized = normalized->getValue().getBool();
    
    ColoredTransformStimulus::draw(display);
    
    last_direction_in_degrees = current_direction_in_degrees;
    last_starting_phase = current_starting_phase;
    last_spatial_frequency = current_spatial_frequency;
    last_speed = current_speed;
    last_inverted = current_inverted;
    last_std_dev = current_std_dev;
    last_mean = current_mean;
    last_normalized = current_normalized;
}


END_NAMESPACE_MW

























