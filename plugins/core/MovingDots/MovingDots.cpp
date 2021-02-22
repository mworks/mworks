/*
 *  MovingDots.cpp
 *  MovingDots
 *
 *  Created by Christopher Stawarz on 8/6/10.
 *  Copyright 2010 MIT. All rights reserved.
 *
 */

#include "MovingDots.h"

using namespace mw::aapl_math_utilities;


BEGIN_NAMESPACE_MW


const std::string MovingDots::FIELD_RADIUS("field_radius");
const std::string MovingDots::FIELD_CENTER_X("field_center_x");
const std::string MovingDots::FIELD_CENTER_Y("field_center_y");
const std::string MovingDots::DOT_DENSITY("dot_density");
const std::string MovingDots::DOT_SIZE("dot_size");
const std::string MovingDots::COLOR("color");
const std::string MovingDots::ALPHA_MULTIPLIER("alpha_multiplier");
const std::string MovingDots::DIRECTION("direction");
const std::string MovingDots::SPEED("speed");
const std::string MovingDots::COHERENCE("coherence");
const std::string MovingDots::LIFETIME("lifetime");
const std::string MovingDots::RAND_SEED("rand_seed");
const std::string MovingDots::MAX_NUM_DOTS("max_num_dots");
const std::string MovingDots::ANNOUNCE_DOTS("announce_dots");


void MovingDots::describeComponent(ComponentInfo &info) {
    MovingDotsBase::describeComponent(info);
    
    info.setSignature("stimulus/moving_dots");
    
    info.addParameter(FIELD_RADIUS);
    info.addParameter(FIELD_CENTER_X, "0.0");
    info.addParameter(FIELD_CENTER_Y, "0.0");
    info.addParameter(DOT_DENSITY);
    info.addParameter(DOT_SIZE);
    info.addParameter(COLOR, "1.0,1.0,1.0");
    info.addParameter(ALPHA_MULTIPLIER, "1.0");
    info.addParameter(DIRECTION, "0.0");
    info.addParameter(SPEED);
    info.addParameter(COHERENCE, "1.0");
    info.addParameter(LIFETIME, "0.0");
    info.addParameter(RAND_SEED, false);
    info.addParameter(MAX_NUM_DOTS, false);
    info.addParameter(ANNOUNCE_DOTS, "NO");
}


MovingDots::MovingDots(const ParameterValueMap &parameters) :
    MovingDotsBase(parameters),
    fieldRadius(registerVariable(parameters[FIELD_RADIUS])),
    fieldCenterX(registerVariable(parameters[FIELD_CENTER_X])),
    fieldCenterY(registerVariable(parameters[FIELD_CENTER_Y])),
    dotDensity(registerVariable(parameters[DOT_DENSITY])),
    dotSize(registerVariable(parameters[DOT_SIZE])),
    alpha(registerVariable(parameters[ALPHA_MULTIPLIER])),
    direction(registerVariable(parameters[DIRECTION])),
    speed(registerVariable(parameters[SPEED])),
    coherence(registerVariable(parameters[COHERENCE])),
    lifetime(registerVariable(parameters[LIFETIME])),
    randSeed(optionalVariable(parameters[RAND_SEED])),
    maxNumDots(optionalVariable(parameters[MAX_NUM_DOTS])),
    announceDots(parameters[ANNOUNCE_DOTS]),
    renderPipelineState(nil),
    bufferPool(nil)
{
    ParsedColorTrio color(parameters[COLOR]);
    red = registerVariable(color.getR());
    green = registerVariable(color.getG());
    blue = registerVariable(color.getB());
}


MovingDots::~MovingDots() {
    @autoreleasepool {
        bufferPool = nil;
        renderPipelineState = nil;
    }
}


Datum MovingDots::getCurrentAnnounceDrawData() {
    boost::mutex::scoped_lock locker(stim_lock);
    
    auto announceData = MovingDotsBase::getCurrentAnnounceDrawData();
    
    announceData.addElement(STIM_TYPE, "moving_dots");
    announceData.addElement(FIELD_RADIUS, currentFieldRadius);
    announceData.addElement(FIELD_CENTER_X, currentFieldCenterX);
    announceData.addElement(FIELD_CENTER_Y, currentFieldCenterY);
    announceData.addElement(DOT_DENSITY, currentDotDensity);
    announceData.addElement(DOT_SIZE, currentDotSize);
    announceData.addElement(STIM_COLOR_R, currentRed);
    announceData.addElement(STIM_COLOR_G, currentGreen);
    announceData.addElement(STIM_COLOR_B, currentBlue);
    announceData.addElement(ALPHA_MULTIPLIER, currentAlpha);
    announceData.addElement(DIRECTION, currentDirection);
    announceData.addElement(SPEED, currentSpeed);
    announceData.addElement(COHERENCE, currentCoherence);
    announceData.addElement(LIFETIME, currentLifetime);
    announceData.addElement(RAND_SEED, currentRandSeed);
    
    announceData.addElement("num_dots", long(currentNumDots));
    if (announceDots->getValue().getBool()) {
        Datum dotsData(reinterpret_cast<char *>(dotPositions.get()), getDotPositionsSize(currentNumDots));
        announceData.addElement("dots", std::move(dotsData));
    }
    
    return announceData;
}


void MovingDots::loadMetal(MetalDisplay &display) {
    boost::mutex::scoped_lock locker(stim_lock);
    
    //
    // Determine maximum number of dots
    //
    {
        if (maxNumDots) {
            currentMaxNumDots = maxNumDots->getValue().getInteger();
            if (currentMaxNumDots < 1) {
                throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Maximum number of dots must be greater than zero");
            }
        } else {
            double newFieldRadius = 0.0;
            double newDotDensity = 0.0;
            if (!computeNumDots(newFieldRadius, newDotDensity, currentMaxNumDots)) {
                throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Cannot compute maximum number of dots");
            }
        }
        
        dotPositions.reset(new simd::float2[currentMaxNumDots]);
        dotDirections.reset(new float[currentMaxNumDots]);
        dotAges.reset(new float[currentMaxNumDots]);
    }
    
    //
    // Seed the random number generator
    //
    if (randSeed) {
        currentRandSeed = randSeed->getValue().getInteger();
    } else {
        currentRandSeed = Clock::instance()->getSystemTimeNS();
    }
    randGen.seed(currentRandSeed);
    
    //
    // Compute conversion from dot size to pixels
    //
    {
        double xMin, xMax, yMin, yMax;
        display.getDisplayBounds(xMin, xMax, yMin, yMax);
        dotSizeToPixels = double(display.getMainView().drawableSize.width) / (xMax - xMin);
    }
    
    //
    // Create render pipeline state
    //
    {
        auto library = loadDefaultLibrary(display, MWORKS_GET_CURRENT_BUNDLE());
        auto vertexFunction = loadShaderFunction(library, "vertexShader");
        auto fragmentFunction = loadShaderFunction(library, "fragmentShader");
        
        auto renderPipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
        renderPipelineDescriptor.vertexFunction = vertexFunction;
        renderPipelineDescriptor.fragmentFunction = fragmentFunction;
        
        auto colorAttachment = renderPipelineDescriptor.colorAttachments[0];
        colorAttachment.pixelFormat = display.getMetalFramebufferTexturePixelFormat();
        colorAttachment.blendingEnabled = YES;
        colorAttachment.sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
        colorAttachment.destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
        colorAttachment.sourceAlphaBlendFactor = MTLBlendFactorSourceAlpha;
        colorAttachment.destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
        
        renderPipelineState = createRenderPipelineState(display, renderPipelineDescriptor);
    }
    
    //
    // Create buffer pool to employ triple buffering, as recommended in the Metal Best Practices Guide:
    // https://developer.apple.com/library/archive/documentation/3DDrawing/Conceptual/MTLBestPracticesGuide/TripleBuffering.html
    //
    {
        NSUInteger length = getDotPositionsSize(currentMaxNumDots);
#if TARGET_OS_OSX
        MTLResourceOptions options = MTLResourceStorageModeManaged;
#else
        MTLResourceOptions options = MTLResourceStorageModeShared;
#endif
        
        bufferPool = [MWKTripleBufferedMTLResource bufferWithDevice:display.getMetalDevice()
                                                             length:length
                                                            options:options];
    }
}


void MovingDots::unloadMetal(MetalDisplay &display) {
    boost::mutex::scoped_lock locker(stim_lock);
    
    bufferPool = nil;
    renderPipelineState = nil;
    
    dotAges.reset();
    dotDirections.reset();
    dotPositions.reset();
}


void MovingDots::drawMetal(MetalDisplay &display) {
    //
    // Update dots
    //
    
    currentTime = getElapsedTime();
    if (previousTime != currentTime) {
        if (!updateParameters()) {
            return;
        }
        updateDots();
        previousTime = currentTime;
    }
    
    //
    // Copy dot positions to the next-available buffer from the pool
    //
    
    auto currentBuffer = [bufferPool acquireWithCommandBuffer:display.getCurrentMetalCommandBuffer()];
    {
        NSUInteger length = getDotPositionsSize(currentNumDots);
        std::memcpy(currentBuffer.contents, dotPositions.get(), length);
#if TARGET_OS_OSX
        [currentBuffer didModifyRange:NSMakeRange(0, length)];
#endif
    }
    
    //
    // Render dots
    //
    
    auto renderCommandEncoder = createRenderCommandEncoder(display);
    [renderCommandEncoder setRenderPipelineState:renderPipelineState];
    
    [renderCommandEncoder setVertexBuffer:currentBuffer offset:0 atIndex:0];
    {
        auto currentMVPMatrix = display.getMetalProjectionMatrix();
        currentMVPMatrix = currentMVPMatrix * matrix4x4_translation(currentFieldCenterX, currentFieldCenterY, 0.0);
        currentMVPMatrix = currentMVPMatrix * matrix4x4_scale(currentFieldRadius, currentFieldRadius, 1.0);
        currentMVPMatrix = currentMVPMatrix * matrix4x4_rotation(radians_from_degrees(currentDirection), 0.0, 0.0, 1.0);
        setVertexBytes(renderCommandEncoder, currentMVPMatrix, 1);
    }
    {
        float pointSize = currentDotSize * dotSizeToPixels;
        setVertexBytes(renderCommandEncoder, pointSize, 2);
    }
    
    {
        auto currentColor = simd::make_float4(currentRed, currentGreen, currentBlue, currentAlpha);
        setFragmentBytes(renderCommandEncoder, currentColor, 0);
    }
    
    [renderCommandEncoder drawPrimitives:MTLPrimitiveTypePoint vertexStart:0 vertexCount:currentNumDots];
    [renderCommandEncoder endEncoding];
}


void MovingDots::drawFrame(boost::shared_ptr<StimulusDisplay> display) {
    MetalStimulus::draw(display);
}


void MovingDots::startPlaying() {
    // With the exception of previousTime, all previous values are overwritten by their
    // current counterparts before being used, so we need to initialize the latter
    previousTime = -1;
    currentFieldRadius = 1.0f;
    currentSpeed = 0.0f;
    currentCoherence = 1.0f;
    currentLifetime = 0.0f;
    currentNumDots = 0;
    
    MovingDotsBase::startPlaying();
}


bool MovingDots::computeNumDots(double &newFieldRadius, double &newDotDensity, std::size_t &newNumDots) const {
    newFieldRadius = fieldRadius->getValue().getFloat();
    if (newFieldRadius <= 0.0) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "Dot field radius must be greater than 0");
        return false;
    }
    
    newDotDensity = dotDensity->getValue().getFloat();
    if (newDotDensity <= 0.0) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "Dot field density must be greater than 0");
        return false;
    }
    
    newNumDots = std::size_t(std::round(newDotDensity * (M_PI * newFieldRadius * newFieldRadius)));
    if (newNumDots < 1) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "Dot field radius and dot density yield 0 dots");
        return false;
    }
    
    return true;
}


bool MovingDots::updateParameters() {
    //
    // Field radius, dot density, and number of dots
    //
    
    double newFieldRadius = 0.0;
    double newDotDensity = 0.0;
    std::size_t newNumDots = 0;
    if (!computeNumDots(newFieldRadius, newDotDensity, newNumDots)) {
        return false;
    }
    if (newNumDots > currentMaxNumDots) {
        merror(M_DISPLAY_MESSAGE_DOMAIN,
               "Current number of dots (%ld) exceeds maximum (%ld).  To resolve this issue, "
               "set %s to an appropriate value.",
               newNumDots,
               currentMaxNumDots,
               MAX_NUM_DOTS.c_str());
        return false;
    }
    
    //
    // Speed
    //
    
    auto newSpeed = speed->getValue().getFloat();
    if (newSpeed < 0.0) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "Dot field speed must be non-negative");
        return false;
    }
    
    //
    // Coherence
    //
    
    auto newCoherence = coherence->getValue().getFloat();
    if ((newCoherence < 0.0) || (newCoherence > 1.0)) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "Dot field coherence must be between 0 and 1");
        return false;
    }
    
    //
    // Lifetime
    //
    
    auto newLifetime = lifetime->getValue().getFloat();
    if (newLifetime < 0.0) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "Dot field lifetime must be non-negative");
        return false;
    }
    
    previousFieldRadius = currentFieldRadius;
    previousSpeed = currentSpeed;
    previousCoherence = currentCoherence;
    previousLifetime = currentLifetime;
    previousNumDots = currentNumDots;
    
    currentFieldRadius = newFieldRadius;
    currentFieldCenterX = fieldCenterX->getValue().getFloat();
    currentFieldCenterY = fieldCenterY->getValue().getFloat();
    currentDotDensity = newDotDensity;
    currentDotSize = dotSize->getValue().getFloat();
    currentRed = red->getValue().getFloat();
    currentGreen = green->getValue().getFloat();
    currentBlue = blue->getValue().getFloat();
    currentAlpha = alpha->getValue().getFloat();
    currentDirection = direction->getValue().getFloat();
    currentSpeed = newSpeed;
    currentCoherence = newCoherence;
    currentLifetime = newLifetime;
    currentNumDots = newNumDots;
    
    return true;
}


void MovingDots::updateDots() {
    //
    // Update positions
    //
    
    const auto numValidDots = std::min(previousNumDots, currentNumDots);
    const auto dt = float(currentTime - previousTime) / 1.0e6f;
    const auto dr = dt * previousSpeed / previousFieldRadius;
    
    for (std::size_t i = 0; i < numValidDots; i++) {
        auto &age = dotAges[i];
        age += dt;
        
        if ((age <= previousLifetime) || (previousLifetime == 0.0f)) {
            advanceDot(i, dr);
        } else {
            replaceDot(i, newDirection(previousCoherence), 0.0f);
        }
    }
    
    //
    // Update directions
    //
    
    if (currentCoherence != previousCoherence) {
        for (std::size_t i = 0; i < numValidDots; i++) {
            dotDirections[i] = newDirection(currentCoherence);
        }
    }
    
    //
    // Update lifetimes
    //
    
    if (currentLifetime != previousLifetime) {
        for (std::size_t i = 0; i < numValidDots; i++) {
            dotAges[i] = newAge(currentLifetime);
        }
    }
    
    //
    // Add dots
    //
    
    if (currentNumDots != previousNumDots) {
        for (std::size_t i = previousNumDots; i < currentNumDots; i++) {
            replaceDot(i, newDirection(currentCoherence), newAge(currentLifetime));
        }
    }
}


void MovingDots::advanceDot(std::size_t i, float dr) {
    auto &pos = dotPositions[i];
    auto &theta = dotDirections[i];
    
    pos.x += dr * std::cos(theta);
    pos.y += dr * std::sin(theta);
    
    if (simd::length_squared(pos) > 1.0f) {
        theta = newDirection(previousCoherence);
        
        float y1 = rand(-1.0f, 1.0f);
        float x1 = -std::sqrt(1.0f - y1*y1) + rand(0.0f, dr);
        
        pos.x = x1*std::cos(theta) - y1*std::sin(theta);
        pos.y = x1*std::sin(theta) + y1*std::cos(theta);
        
        dotAges[i] = newAge(previousLifetime);
    }
}


void MovingDots::replaceDot(std::size_t i, float direction, float age) {
    auto &pos = dotPositions[i];
    
    do {
        pos.x = rand(-1.0f, 1.0f);
        pos.y = rand(-1.0f, 1.0f);
    } while (simd::length_squared(pos) > 1.0f);
    
    dotDirections[i] = direction;
    dotAges[i] = age;
}


END_NAMESPACE_MW
