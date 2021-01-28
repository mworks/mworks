/*
 *  DriftingGratingStimulus.cpp
 *  MWorksCore
 *
 *  Created by bkennedy on 8/26/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#include "DriftingGratingStimulus.h"

using namespace mw::aapl_math_utilities;


BEGIN_NAMESPACE_MW


const std::string DriftingGratingStimulus::DIRECTION("direction");
const std::string DriftingGratingStimulus::CENTRAL_STARTING_PHASE("central_starting_phase");
const std::string DriftingGratingStimulus::STARTING_PHASE("starting_phase");
const std::string DriftingGratingStimulus::FREQUENCY("spatial_frequency");
const std::string DriftingGratingStimulus::SPEED("speed");
const std::string DriftingGratingStimulus::COMPUTE_PHASE_INCREMENTALLY("compute_phase_incrementally");
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
    info.addParameter(CENTRAL_STARTING_PHASE, false);
    info.addParameter(STARTING_PHASE, false);
    info.addParameter(FREQUENCY);
    info.addParameter(SPEED);
    info.addParameter(COMPUTE_PHASE_INCREMENTALLY, "NO");
    info.addParameter(GRATING_TYPE);
    info.addParameter(MASK, "NO");
    info.addParameter(INVERTED, "NO");
    info.addParameter(STD_DEV, "1.0");
    info.addParameter(MEAN, "0.0");
    info.addParameter(NORMALIZED, "YES");
    
    // This is no longer used but may be present in old experiments
    info.addIgnoredParameter("grating_sample_rate");
}


DriftingGratingStimulus::DriftingGratingStimulus(const ParameterValueMap &parameters) :
    DriftingGratingStimulusBase(parameters),
    direction_in_degrees(registerVariable(parameters[DIRECTION])),
    central_starting_phase(registerOptionalVariable(optionalVariable(parameters[CENTRAL_STARTING_PHASE]))),
    starting_phase(registerOptionalVariable(optionalVariable(parameters[STARTING_PHASE]))),
    spatial_frequency(registerVariable(parameters[FREQUENCY])),
    speed(registerVariable(parameters[SPEED])),
    computePhaseIncrementally(parameters[COMPUTE_PHASE_INCREMENTALLY]),
    gratingTypeName(registerVariable(variableOrText(parameters[GRATING_TYPE]))),
    maskTypeName(registerVariable(variableOrText(parameters[MASK]))),
    inverted(registerVariable(parameters[INVERTED])),
    std_dev(registerVariable(parameters[STD_DEV])),
    mean(registerVariable(parameters[MEAN])),
    normalized(registerVariable(parameters[NORMALIZED])),
    lastElapsedSeconds(-1.0),
    displayWidth(0.0),
    displayHeight(0.0)
{
    if (central_starting_phase && starting_phase) {
        throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, (boost::format("Only one of %1% and %2% may be specified")
                                                         % CENTRAL_STARTING_PHASE
                                                         % STARTING_PHASE));
    }
}


Datum DriftingGratingStimulus::getCurrentAnnounceDrawData() {
    boost::mutex::scoped_lock locker(stim_lock);
    
    auto announceData = DriftingGratingStimulusBase::getCurrentAnnounceDrawData();
    
    announceData.addElement(STIM_TYPE, "drifting_grating");
    announceData.addElement(DIRECTION, current_direction_in_degrees);
    announceData.addElement((central_starting_phase ? CENTRAL_STARTING_PHASE : STARTING_PHASE), current_starting_phase);
    announceData.addElement("current_phase", last_phase*(180/M_PI));
    announceData.addElement(FREQUENCY, current_spatial_frequency);
    announceData.addElement(SPEED, current_speed);
    announceData.addElement(COMPUTE_PHASE_INCREMENTALLY, computePhaseIncrementally);
    announceData.addElement(GRATING_TYPE, current_grating_type_name);
    
    if (!current_mask_type_name.empty()) {
        announceData.addElement(MASK, current_mask_type_name);
    } else {
        announceData.addElement(MASK, current_grating_is_mask);
    }
    
    announceData.addElement(INVERTED, current_inverted);
    
    if (current_mask_type == MaskType::gaussian) {
        announceData.addElement(STD_DEV, current_std_dev);
        announceData.addElement(MEAN, current_mean);
        announceData.addElement(NORMALIZED, current_normalized);
    }
    
    return announceData;
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
    merror(M_DISPLAY_MESSAGE_DOMAIN, "Invalid grating type: %s", name.c_str());
    return GratingType::sinusoid;
};


auto DriftingGratingStimulus::maskTypeFromName(const std::string &name) -> MaskType {
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


void DriftingGratingStimulus::loadMetal(MetalDisplay &display) {
    boost::mutex::scoped_lock locker(stim_lock);
    
    ColoredTransformStimulus::loadMetal(display);
    
    // Whether or not the grating is a mask must be determined at load time, because that's when
    // blending is configured
    {
        auto maskType = maskTypeName->getValue();
        if (maskType.isString()) {
            current_grating_is_mask = false;
        } else {
            current_mask_type_name.clear();
            current_mask_type = MaskType::rectangle;
            current_grating_is_mask = maskType.getBool();
        }
    }
    
    if (fullscreen) {
        double xMin, xMax, yMin, yMax;
        display.getDisplayBounds(xMin, xMax, yMin, yMax);
        displayWidth = xMax - xMin;
        displayHeight = yMax - yMin;
    }
    
    auto library = loadDefaultLibrary(display, MWORKS_GET_CURRENT_BUNDLE());
    
    MTLFunctionConstantValues *constantValues = [[MTLFunctionConstantValues alloc] init];
    const bool nonMaskGrating = !current_grating_is_mask;
    [constantValues setConstantValue:&nonMaskGrating
                                type:MTLDataTypeBool
                             atIndex:nonMaskGratingFunctionConstantIndex];
    
    auto vertexFunction = loadShaderFunction(library, "vertexShader", constantValues);
    auto fragmentFunction = loadShaderFunction(library, "fragmentShader", constantValues);
    auto renderPipelineDescriptor = createRenderPipelineDescriptor(display, vertexFunction, fragmentFunction);
    renderPipelineState = createRenderPipelineState(display, renderPipelineDescriptor);
}


void DriftingGratingStimulus::drawMetal(MetalDisplay &display) {
    //
    // Get current parameter values
    //
    
    ColoredTransformStimulus::drawMetal(display);
    
    current_direction_in_degrees = direction_in_degrees->getValue().getFloat();
    if (central_starting_phase) {
        current_starting_phase = central_starting_phase->getValue().getFloat();
    } else if (starting_phase) {
        current_starting_phase = starting_phase->getValue().getFloat();
    } else {
        current_starting_phase = 0.0;
    }
    current_spatial_frequency = spatial_frequency->getValue().getFloat();
    current_speed = speed->getValue().getFloat();
    current_grating_type_name = gratingTypeName->getValue().getString();
    current_grating_type = gratingTypeFromName(current_grating_type_name);
    if (!current_grating_is_mask) {
        auto maskType = maskTypeName->getValue();
        if (maskType.isString()) {
            current_mask_type_name = maskType.getString();
            current_mask_type = maskTypeFromName(current_mask_type_name);
        } else {
            current_mask_type_name.clear();
            current_mask_type = MaskType::rectangle;
            if (maskType.getBool()) {
                merror(M_DISPLAY_MESSAGE_DOMAIN, "Grating cannot be changed into a mask after loading");
            }
        }
    }
    current_inverted = inverted->getValue().getBool();
    current_std_dev = std_dev->getValue().getFloat();
    current_mean = mean->getValue().getFloat();
    current_normalized = normalized->getValue().getBool();
    
    if (fullscreen) {
        current_sizex = displayWidth;
        current_sizey = displayHeight;
    }
    
    //
    // Compute current grating state
    //
    
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
    const double elapsed_seconds = double(getElapsedTime()) / 1000000.0;
    double phase = 2.0 * M_PI * current_speed * current_spatial_frequency;
    if (!computePhaseIncrementally || (lastElapsedSeconds <= 0.0)) {
        phase = -((M_PI / 180.0) * current_starting_phase + phase * elapsed_seconds);
    } else {
        phase = last_phase - phase * (elapsed_seconds - lastElapsedSeconds);
    }
    
    const double direction_in_radians = current_direction_in_degrees*(M_PI/180.);
    const float f = std::cos(direction_in_radians);
    const float g = std::sin(direction_in_radians);
    const float d = ((f+g)-1)/2;
    const float grating_bl = 0-d;
    const float grating_br = grating_bl+f;
    const float grating_tr = 1+d;
    const float grating_tl = grating_bl+g;
    
    const float cycle_proportion = current_spatial_frequency * std::max(current_sizex, current_sizey);
    const float phase_offset = (central_starting_phase ? M_PI * cycle_proportion : 0.0);
    const float phase_proportion = (phase - phase_offset) / (2.0 * M_PI);
    
    const float gratingCoords[] = {
        (cycle_proportion * grating_bl) + phase_proportion,
        (cycle_proportion * grating_br) + phase_proportion,
        (cycle_proportion * grating_tl) + phase_proportion,
        (cycle_proportion * grating_tr) + phase_proportion
    };
    
    lastElapsedSeconds = elapsed_seconds;
    last_phase = phase;
    
    //
    // Encode rendering commands
    //
    
    auto renderCommandEncoder = createRenderCommandEncoder(display);
    [renderCommandEncoder setRenderPipelineState:renderPipelineState];
    
    setCurrentMVPMatrix(display, renderCommandEncoder, mvpMatrixBufferIndex);
    setVertexBytes(renderCommandEncoder, gratingCoords, gratingCoordsBufferIndex);
    if (!current_grating_is_mask) {
        float aspectRatio = current_sizex / current_sizey;
        setVertexBytes(renderCommandEncoder, aspectRatio, aspectRatioBufferIndex);
    }
    
    GratingParameters gratingParams;
    gratingParams.gratingType = current_grating_type;
    gratingParams.inverted = current_inverted;
    setFragmentBytes(renderCommandEncoder, gratingParams, gratingParamsBufferIndex);
    
    if (!current_grating_is_mask) {
        MaskParameters maskParams;
        maskParams.maskType = current_mask_type;
        maskParams.stdDev = current_std_dev;
        maskParams.mean = current_mean;
        maskParams.normalized = current_normalized;
        setFragmentBytes(renderCommandEncoder, maskParams, maskParamsBufferIndex);
        
        setCurrentColor(renderCommandEncoder, colorBufferIndex);
    }
    
    [renderCommandEncoder drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];
    [renderCommandEncoder endEncoding];
}


void DriftingGratingStimulus::configureBlending(MTLRenderPipelineColorAttachmentDescriptor *colorAttachment) const {
    if (!current_grating_is_mask) {
        ColoredTransformStimulus::configureBlending(colorAttachment);
    } else {
        // Blend by multiplying the destination alpha by the grating alpha
        colorAttachment.blendingEnabled = YES;
        colorAttachment.sourceRGBBlendFactor = MTLBlendFactorZero;
        colorAttachment.destinationRGBBlendFactor = MTLBlendFactorOne;
        colorAttachment.sourceAlphaBlendFactor = MTLBlendFactorZero;
        colorAttachment.destinationAlphaBlendFactor = MTLBlendFactorSourceAlpha;
    }
}


simd::float4x4 DriftingGratingStimulus::getCurrentMVPMatrix(const simd::float4x4 &projectionMatrix) const {
    auto currentMVPMatrix = projectionMatrix;
    if (!fullscreen) {
        currentMVPMatrix = currentMVPMatrix * matrix4x4_translation(current_posx, current_posy, 0.0);
        currentMVPMatrix = currentMVPMatrix * matrix4x4_rotation(radians_from_degrees(current_rot), 0.0, 0.0, 1.0);
    }
    auto scale_size = std::max(current_sizex, current_sizey);
    currentMVPMatrix = currentMVPMatrix * matrix4x4_scale(scale_size, scale_size, 1.0);
    return currentMVPMatrix;
}


void DriftingGratingStimulus::drawFrame(boost::shared_ptr<StimulusDisplay> display) {
    ColoredTransformStimulus::draw(display);
}


void DriftingGratingStimulus::stopPlaying() {
    DriftingGratingStimulusBase::stopPlaying();
    lastElapsedSeconds = -1.0;
}


END_NAMESPACE_MW
