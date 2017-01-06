/*
 *  DriftingGratingStimulus.cpp
 *  MWorksCore
 *
 *  Created by bkennedy on 8/26/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#include "DriftingGratingStimulus.h"
#include "EllipseMask.h"
#include "GaussianMask.h"
#include "RectangleMask.h"
#include "SawtoothGratingData.h"
#include "SinusoidGratingData.h"
#include "SquareGratingData.h"
#include "TriangleGratingData.h"


BEGIN_NAMESPACE_MW


const std::string DriftingGratingStimulus::DIRECTION("direction");
const std::string DriftingGratingStimulus::STARTING_PHASE("starting_phase");
const std::string DriftingGratingStimulus::FREQUENCY("spatial_frequency");
const std::string DriftingGratingStimulus::SPEED("speed");
const std::string DriftingGratingStimulus::GRATING_TYPE("grating_type");
const std::string DriftingGratingStimulus::MASK("mask");
const std::string DriftingGratingStimulus::GRATING_SAMPLE_RATE("grating_sample_rate");
const std::string DriftingGratingStimulus::INVERTED("inverted");
const std::string DriftingGratingStimulus::STD_DEV("std_dev");
const std::string DriftingGratingStimulus::MEAN("mean");


void DriftingGratingStimulus::describeComponent(ComponentInfo &info) {
    DriftingGratingStimulusBase::describeComponent(info);
    
    info.setSignature("stimulus/drifting_grating");
    
    info.addParameter(DIRECTION, "0.0");
    info.addParameter(STARTING_PHASE, "0.0");
    info.addParameter(FREQUENCY);
    info.addParameter(SPEED);
    info.addParameter(GRATING_TYPE);
    info.addParameter(MASK);
    info.addParameter(GRATING_SAMPLE_RATE, "32");
    info.addParameter(INVERTED, "0");
    info.addParameter(STD_DEV, "1.0");
    info.addParameter(MEAN, "0.0");
}


DriftingGratingStimulus::DriftingGratingStimulus(const ParameterValueMap &parameters) :
    DriftingGratingStimulusBase(parameters),
    direction_in_degrees(registerVariable(parameters[DIRECTION])),
    spatial_frequency(registerVariable(parameters[FREQUENCY])),
    speed(registerVariable(parameters[SPEED])),
    starting_phase(registerVariable(parameters[STARTING_PHASE]))
{
    const std::string &grating_type = parameters[GRATING_TYPE].str();
    shared_ptr<Variable> grating_sample_rate(parameters[GRATING_SAMPLE_RATE]);
    
    if (grating_type == "sinusoid") {
        grating = shared_ptr<SinusoidGratingData>(new SinusoidGratingData(grating_sample_rate));
    } else if (grating_type == "square") {
        grating = shared_ptr<SquareGratingData>(new SquareGratingData(grating_sample_rate));
    } else if (grating_type == "triangle") {
        grating = shared_ptr<TriangleGratingData>(new TriangleGratingData(grating_sample_rate));
    } else if (grating_type == "sawtooth") {
        grating = shared_ptr<SawtoothGratingData>(new SawtoothGratingData(grating_sample_rate,
                                                                          VariablePtr(parameters[INVERTED])));
    } else {
        throw SimpleException("illegal grating type", grating_type);
    }
    
    const std::string &mask_type = parameters[MASK].str();
    shared_ptr<Variable> mask_size(new ConstantVariable(128L));
    
    if (mask_type == "rectangle") {
        mask = shared_ptr<Mask>(new RectangleMask(mask_size));
    } else if (mask_type == "ellipse") {
        mask = shared_ptr<Mask>(new EllipseMask(mask_size));
    } else if (mask_type == "gaussian") {
        mask = shared_ptr<Mask>(new GaussianMask(mask_size,
                                                 VariablePtr(parameters[MEAN]),
                                                 VariablePtr(parameters[STD_DEV])));
    } else {
        throw SimpleException("illegal mask", mask_type);				
    }
}


Datum DriftingGratingStimulus::getCurrentAnnounceDrawData() {
    boost::mutex::scoped_lock locker(stim_lock);
    
    Datum announceData = DriftingGratingStimulusBase::getCurrentAnnounceDrawData();
    
    announceData.addElement(STIM_TYPE, "drifting_grating");
    announceData.addElement("frequency", spatial_frequency->getValue());
    announceData.addElement("speed", speed->getValue());
    announceData.addElement("starting_phase", starting_phase->getValue());
    announceData.addElement("current_phase", last_phase);
    announceData.addElement("direction", direction_in_degrees->getValue());
    announceData.addElement("grating", grating->getName());
    announceData.addElement("mask", mask->getName());
    
    return std::move(announceData);
}


gl::Shader DriftingGratingStimulus::getVertexShader() const {
    static const std::string source
    (R"(
     uniform mat4 mvpMatrix;
     
     in vec4 vertexPosition;
     in float gratingTexCoord;
     in vec2 maskTexCoords;
     
     out float gratingVaryingTexCoord;
     out vec2 maskVaryingTexCoords;
     
     void main() {
         gl_Position = mvpMatrix * vertexPosition;
         gratingVaryingTexCoord = gratingTexCoord;
         maskVaryingTexCoords = maskTexCoords;
     }
     )");
    
    return gl::createShader(GL_VERTEX_SHADER, source);
}


gl::Shader DriftingGratingStimulus::getFragmentShader() const {
    static const std::string source
    (R"(
     uniform float alpha;
     uniform sampler1D gratingTexture;
     uniform sampler2D maskTexture;
     
     in float gratingVaryingTexCoord;
     in vec2 maskVaryingTexCoords;
     
     out vec4 fragColor;
     
     void main() {
         float gratingValue = texture(gratingTexture, gratingVaryingTexCoord).r;
         float maskValue = texture(maskTexture, maskVaryingTexCoords).r;
         fragColor = vec4(gratingValue, gratingValue, gratingValue, alpha * maskValue);
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
    
    BasicTransformStimulus::prepare(display);
    
    alphaUniformLocation = glGetUniformLocation(program, "alpha");
    
    glUniform1i(glGetUniformLocation(program, "gratingTexture"), 0);
    glUniform1i(glGetUniformLocation(program, "maskTexture"), 1);
    
    glGenBuffers(1, &gratingTexCoordBuffer);
    gl::BufferBinding<GL_ARRAY_BUFFER> arrayBufferBinding(gratingTexCoordBuffer);
    GLint gratingTexCoordAttribLocation = glGetAttribLocation(program, "gratingTexCoord");
    glEnableVertexAttribArray(gratingTexCoordAttribLocation);
    glVertexAttribPointer(gratingTexCoordAttribLocation, 1, GL_FLOAT, GL_FALSE, 0, nullptr);
    
    glGenBuffers(1, &maskTexCoordsBuffer);
    arrayBufferBinding.bind(maskTexCoordsBuffer);
    GLint maskTexCoordsAttribLocation = glGetAttribLocation(program, "maskTexCoords");
    glEnableVertexAttribArray(maskTexCoordsAttribLocation);
    glVertexAttribPointer(maskTexCoordsAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    
    // ----------------------------------------
    //                  GRATING
    // ----------------------------------------
    
    glGenTextures(1, &gratingTexture);
    gl::TextureBinding<GL_TEXTURE_1D> texture1DBinding(gratingTexture);
    
    glTexImage1D(GL_TEXTURE_1D,
                 0,
                 GL_RED,
                 grating->getDataSize(),
                 0,
                 GL_RED,
                 GL_FLOAT,
                 grating->get1DData());
    glGenerateMipmap(GL_TEXTURE_1D);
    
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    // ----------------------------------------
    //                  MASK
    // ----------------------------------------
    
    glGenTextures(1, &maskTexture);
    gl::TextureBinding<GL_TEXTURE_2D> texture2DBinding(maskTexture);
    
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RED,
                 mask->getSize(),
                 mask->getSize(),
                 0,
                 GL_RED,
                 GL_FLOAT,
                 mask->get2DData());
    glGenerateMipmap(GL_TEXTURE_2D);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}


void DriftingGratingStimulus::destroy(const boost::shared_ptr<StimulusDisplay> &display) {
    boost::mutex::scoped_lock locker(stim_lock);
    
    glDeleteTextures(1, &maskTexture);
    glDeleteTextures(1, &gratingTexture);
    glDeleteBuffers(1, &maskTexCoordsBuffer);
    glDeleteBuffers(1, &gratingTexCoordBuffer);
    
    BasicTransformStimulus::destroy(display);
}


void DriftingGratingStimulus::preDraw(const boost::shared_ptr<StimulusDisplay> &display) {
    BasicTransformStimulus::preDraw(display);
    
    glUniform1f(alphaUniformLocation, current_alpha);
    
    glBindTexture(GL_TEXTURE_1D, gratingTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, maskTexture);
    
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
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
    const double phase = -1*(starting_phase->getValue().getFloat()*(M_PI/180.) + speed->getValue().getFloat()*spatial_frequency->getValue().getFloat()*(2.*M_PI)*elapsed_seconds);
    const double direction_in_radians = direction_in_degrees->getValue().getFloat()*(M_PI/180.);
    const double aspect = current_sizex / current_sizey;
    
    const float f = std::cos(direction_in_radians);
    const float g = std::sin(direction_in_radians);
    const float d = ((f+g)-1)/2;
    const float texture_bl = 0-d;
    const float texture_br = texture_bl+f;
    const float texture_tr = 1+d;
    const float texture_tl = texture_bl+g;
    
    const float mask_s_ratio = 1-std::min(1.0,aspect);
    const float mask_t_ratio = 1-std::min(1.0,1.0/aspect);
    
    const float phase_proportion = phase/(2*M_PI);
    const float cycle_proportion = spatial_frequency->getValue().getFloat() * std::max(current_sizex, current_sizey);
    
    std::array<GLfloat, 4 * 1> gratingTexCoord = {
        (cycle_proportion * texture_bl) + phase_proportion,
        (cycle_proportion * texture_br) + phase_proportion,
        (cycle_proportion * texture_tl) + phase_proportion,
        (cycle_proportion * texture_tr) + phase_proportion
    };
    gl::BufferBinding<GL_ARRAY_BUFFER> arrayBufferBinding(gratingTexCoordBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gratingTexCoord), gratingTexCoord.data(), GL_STREAM_DRAW);
    
    std::array<GLfloat, 4 * 2> maskTexCoords = {
        0 - mask_s_ratio, 0 - mask_t_ratio,
        1 + mask_s_ratio, 0 - mask_t_ratio,
        0 - mask_s_ratio, 1 + mask_t_ratio,
        1 + mask_s_ratio, 1 + mask_t_ratio
    };
    arrayBufferBinding.bind(maskTexCoordsBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(maskTexCoords), maskTexCoords.data(), GL_STREAM_DRAW);
    
    last_phase = phase*(180/M_PI);
}


void DriftingGratingStimulus::postDraw(const boost::shared_ptr<StimulusDisplay> &display) {
    glDisable(GL_BLEND);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_1D, 0);
    
    BasicTransformStimulus::postDraw(display);
}


void DriftingGratingStimulus::drawFrame(boost::shared_ptr<StimulusDisplay> display) {
    BasicTransformStimulus::draw(display);
}


END_NAMESPACE_MW

























