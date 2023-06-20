//
//  OpticFlowField.cpp
//  OpticFlowField
//
//  Created by Christopher Stawarz on 5/31/23.
//

#include "OpticFlowField.hpp"

using namespace mw::aapl_math_utilities;


BEGIN_NAMESPACE_MW


BEGIN_NAMESPACE()


template <typename Vector>
auto getRotationMatrix(float radians, float x, float y, float z);


template <>
auto getRotationMatrix<simd::float3>(float radians, float x, float y, float z) {
    return matrix3x3_rotation(radians, x, y, z);
}


template <>
auto getRotationMatrix<simd::float4>(float radians, float x, float y, float z) {
    return matrix4x4_rotation(radians, x, y, z);
}


template <typename Vector>
auto getRotationMatrix(float dirAltitude, float dirAzimuth) {
    return simd_mul(getRotationMatrix<Vector>(radians_from_degrees(dirAzimuth), 0.0f, -1.0f, 0.0f),
                    getRotationMatrix<Vector>(radians_from_degrees(dirAltitude), 1.0f, 0.0f, 0.0f));
}


template <typename Vector>
Vector rotateVector(Vector vect, float dirAltitude, float dirAzimuth) {
    return simd_mul(getRotationMatrix<Vector>(dirAltitude, dirAzimuth), vect);
}


END_NAMESPACE()


const std::string OpticFlowField::Z_NEAR("z_near");
const std::string OpticFlowField::Z_FAR("z_far");
const std::string OpticFlowField::DOT_DENSITY("dot_density");
const std::string OpticFlowField::DOT_SIZE("dot_size");
const std::string OpticFlowField::DIRECTION_ALTITUDE("direction_altitude");
const std::string OpticFlowField::DIRECTION_AZIMUTH("direction_azimuth");
const std::string OpticFlowField::SPEED("speed");
const std::string OpticFlowField::COHERENCE("coherence");
const std::string OpticFlowField::LIFETIME("lifetime");
const std::string OpticFlowField::MAX_NUM_DOTS("max_num_dots");
const std::string OpticFlowField::RAND_SEED("rand_seed");


void OpticFlowField::describeComponent(ComponentInfo &info) {
    OpticFlowFieldBase::describeComponent(info);
    
    info.setSignature("stimulus/optic_flow_field");
    
    info.addParameter(Z_NEAR);
    info.addParameter(Z_FAR);
    info.addParameter(DOT_DENSITY);
    info.addParameter(DOT_SIZE);
    info.addParameter(DIRECTION_ALTITUDE, "0.0");
    info.addParameter(DIRECTION_AZIMUTH, "0.0");
    info.addParameter(SPEED);
    info.addParameter(COHERENCE, "1.0");
    info.addParameter(LIFETIME, "0.0");
    info.addParameter(MAX_NUM_DOTS, false);
    info.addParameter(RAND_SEED, false);
}


OpticFlowField::OpticFlowField(const ParameterValueMap &parameters) :
    OpticFlowFieldBase(parameters),
    zNear(registerVariable(parameters[Z_NEAR])),
    zFar(registerVariable(parameters[Z_FAR])),
    dotDensity(registerVariable(parameters[DOT_DENSITY])),
    dotSize(registerVariable(parameters[DOT_SIZE])),
    directionAltitude(registerVariable(parameters[DIRECTION_ALTITUDE])),
    directionAzimuth(registerVariable(parameters[DIRECTION_AZIMUTH])),
    speed(registerVariable(parameters[SPEED])),
    coherence(registerVariable(parameters[COHERENCE])),
    lifetime(registerVariable(parameters[LIFETIME])),
    maxNumDots(optionalVariable(parameters[MAX_NUM_DOTS])),
    randSeed(optionalVariable(parameters[RAND_SEED])),
    bufferPool(nil)
{ }


OpticFlowField::~OpticFlowField() {
    @autoreleasepool {
        bufferPool = nil;
    }
}


Datum OpticFlowField::getCurrentAnnounceDrawData() {
    boost::mutex::scoped_lock locker(stim_lock);
    
    auto announceData = OpticFlowFieldBase::getCurrentAnnounceDrawData();
    
    announceData.addElement(STIM_TYPE, "optic_flow_field");
    
    announceData.addElement(Z_NEAR, currentZNear);
    announceData.addElement(Z_FAR, currentZFar);
    announceData.addElement(DOT_DENSITY, currentDotDensity);
    announceData.addElement(DOT_SIZE, currentDotSize);
    announceData.addElement(DIRECTION_ALTITUDE, currentDirectionAltitude);
    announceData.addElement(DIRECTION_AZIMUTH, currentDirectionAzimuth);
    announceData.addElement(SPEED, currentSpeed);
    announceData.addElement(COHERENCE, currentCoherence);
    announceData.addElement(LIFETIME, currentLifetime);
    announceData.addElement(RAND_SEED, currentRandSeed);
    
    return announceData;
}


void OpticFlowField::loadMetal(MetalDisplay &display) {
    boost::mutex::scoped_lock locker(stim_lock);
    
    ColoredTransformStimulus::loadMetal(display);
    
    //
    // Get display dimensions and compute conversion from dot size to pixels
    //
    {
        double xMin, xMax, yMin, yMax;
        display.getDisplayBounds(xMin, xMax, yMin, yMax);
        displayWidth = xMax - xMin;
        displayHeight = yMax - yMin;
        dotSizeToPixels = double(display.getMainView().drawableSize.width) / (xMax - xMin);
    }
    
    //
    // Determine maximum number of dots
    //
    if (maxNumDots) {
        currentMaxNumDots = maxNumDots->getValue().getInteger();
        if (currentMaxNumDots < 1) {
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Maximum number of dots must be greater than zero");
        }
    } else {
        float width, height;
        if (!fullscreen) {
            getCurrentSize(width, height);
        } else {
            width = displayWidth;
            height = displayHeight;
        }
        if (!computeNumDots(width, height, currentMaxNumDots)) {
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Cannot compute maximum number of dots");
        }
    }
    
    //
    // Allocate dot data storage
    //
    dotPositions.reset(new simd::float4[currentMaxNumDots]);
    dotDirections.reset(new Direction[currentMaxNumDots]);
    dotAges.reset(new float[currentMaxNumDots]);
    
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
    // Create render pipeline state
    //
    {
        auto library = loadDefaultLibrary(display, MWORKS_GET_CURRENT_BUNDLE());
        auto vertexFunction = loadShaderFunction(library, "vertexShader");
        auto fragmentFunction = loadShaderFunction(library, "fragmentShader");
        auto renderPipelineDescriptor = createRenderPipelineDescriptor(display, vertexFunction, fragmentFunction);
        
        // Mark vertex buffers as immutable, as recommended at
        // https://developer.apple.com/documentation/metal/resource_synchronization/synchronizing_cpu_and_gpu_work
        renderPipelineDescriptor.vertexBuffers[0].mutability = MTLMutabilityImmutable;
        
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


void OpticFlowField::unloadMetal(MetalDisplay &display) {
    boost::mutex::scoped_lock locker(stim_lock);
    
    bufferPool = nil;
    
    dotAges.reset();
    dotDirections.reset();
    dotPositions.reset();
    
    ColoredTransformStimulus::unloadMetal(display);
}


void OpticFlowField::drawMetal(MetalDisplay &display) {
    ColoredTransformStimulus::drawMetal(display);
    
    //
    // Update dots
    //
    
    currentTime = getElapsedTime();
    if (previousTime != currentTime) {
        if (!updateParameters()) {
            // Invalidate all dots
            currentNumDots = 0;
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
        std::memcpy([currentBuffer contents], dotPositions.get(), length);
#if TARGET_OS_OSX
        [currentBuffer didModifyRange:NSMakeRange(0, length)];
#endif
    }
    
    //
    // Encode rendering commands
    //
    
    auto renderCommandEncoder = createRenderCommandEncoder(display);
    [renderCommandEncoder setRenderPipelineState:renderPipelineState];
    
    [renderCommandEncoder setVertexBuffer:currentBuffer offset:0 atIndex:0];
    {
        // SDP = scale, direction, perspective
        auto sdpMatrix = simd_mul(currentPerspectiveMatrix, getRotationMatrix<simd::float4>(currentDirectionAltitude,
                                                                                            currentDirectionAzimuth));
        sdpMatrix = simd_mul(sdpMatrix, matrix4x4_scale(currentFieldRadius, currentFieldRadius, currentFieldRadius));
        setVertexBytes(renderCommandEncoder, sdpMatrix, 1);
    }
    setCurrentMVPMatrix(display, renderCommandEncoder, 2);
    {
        float pointSize = currentDotSize * dotSizeToPixels;
        setVertexBytes(renderCommandEncoder, pointSize, 3);
    }
    
    setCurrentColor(renderCommandEncoder, 0);
    
    [renderCommandEncoder drawPrimitives:MTLPrimitiveTypePoint vertexStart:0 vertexCount:currentNumDots];
    [renderCommandEncoder endEncoding];
}


simd::float4x4 OpticFlowField::getCurrentMVPMatrix(const simd::float4x4 &projectionMatrix) const {
    auto currentMVPMatrix = projectionMatrix;
    if (!fullscreen) {
        currentMVPMatrix = currentMVPMatrix * matrix4x4_translation(current_posx, current_posy, 0.0);
        currentMVPMatrix = currentMVPMatrix * matrix4x4_rotation(radians_from_degrees(current_rot), 0.0, 0.0, 1.0);
    }
    currentMVPMatrix = currentMVPMatrix * matrix4x4_scale(0.5 * current_sizex, 0.5 * current_sizey, 1.0);
    return currentMVPMatrix;
}


void OpticFlowField::startPlaying() {
    // With the exception of previousTime, all previous values are overwritten by their
    // current counterparts before being used, so we need to initialize the latter
    previousTime = -1;
    currentSpeed = 0.0f;
    currentCoherence = 1.0f;
    currentLifetime = 0.0f;
    currentFieldRadius = 0.0f;
    currentNumDots = 0;
    
    OpticFlowFieldBase::startPlaying();
}


bool OpticFlowField::computeNumDots(double newWidth, double newHeight, std::size_t &newNumDots) const {
    double newZNear, newZFar, newFieldRadius;
    simd::float4x4 newPerspectiveMatrix;
    double newDotDensity;
    return computeNumDots(newWidth,
                          newHeight,
                          newZNear,
                          newZFar,
                          newFieldRadius,
                          newPerspectiveMatrix,
                          newDotDensity,
                          newNumDots);
}


bool OpticFlowField::computeNumDots(double newWidth,
                                    double newHeight,
                                    double &newZNear,
                                    double &newZFar,
                                    double &newFieldRadius,
                                    simd::float4x4 &newPerspectiveMatrix,
                                    double &newDotDensity,
                                    std::size_t &newNumDots) const
{
    const auto newHalfWidth = newWidth / 2.0;
    const auto newHalfHeight = newHeight / 2.0;
    
    newZNear = zNear->getValue().getFloat();
    newZFar = zFar->getValue().getFloat();
    if (newZNear <= 0.0 || newZFar <= 0.0) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "Z-near and Z-far must be greater than 0");
        return false;
    }
    if (newZNear >= newZFar) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "Z-near must be less than Z-far");
        return false;
    }
    
    newFieldRadius = simd::length(simd::make_float3(newHalfWidth * newZFar / newZNear,
                                                    newHalfHeight * newZFar / newZNear,
                                                    newZFar));
    
    newPerspectiveMatrix = matrix_perspective_frustum_right_hand(-newHalfWidth,
                                                                 +newHalfWidth,
                                                                 -newHalfHeight,
                                                                 +newHalfHeight,
                                                                 newZNear,
                                                                 newZFar);

    newDotDensity = dotDensity->getValue().getFloat();
    if (newDotDensity <= 0.0) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "Dot density must be greater than 0");
        return false;
    }
    
    //
    // Dot density is the desired number of dots visible on screen per unit screen area
    // (i.e. dots per square degree).  Because the dot field is spherical, we need to
    // convert this to dots per cubic degree in order to compute the total number of dots.
    //
    // To do this, we note that the region of the sphere that is visible at any time
    // is the viewing frustum.  The desired number of dots in the frustum is
    // newWidth*newHeight*newDotDensity.  Dividing this number by the volume of the frustum
    // gives us the desired dots per cubic degree.  We then multiply this by the volume of
    // the sphere to get the total number of dots.
    //
    
    const auto frustumNearFaceArea = newWidth * newHeight;
    const auto frustumFarFaceArea = (2.0 * newHalfWidth * newZFar / newZNear) * (2.0 * newHalfHeight * newZFar / newZNear);
    const auto frustumVolume = (newZFar * frustumFarFaceArea - newZNear * frustumNearFaceArea) / 3.0;
    const auto dotsPerUnitVolume = (newWidth * newHeight * newDotDensity) / frustumVolume;
    
    newNumDots = std::size_t(std::round((4.0 / 3.0) * std::numbers::pi
                                        * newFieldRadius * newFieldRadius * newFieldRadius
                                        * dotsPerUnitVolume));
    if (newNumDots < 1) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "Field size and dot density yield 0 dots");
        return false;
    }
    
    return true;
}


bool OpticFlowField::updateParameters() {
    //
    // Z-near, Z-far, field radius, perspective matrix, dot density, and number of dots
    //
    
    if (fullscreen) {
        current_sizex = displayWidth;
        current_sizey = displayHeight;
    }
    
    double newZNear, newZFar, newFieldRadius;
    simd::float4x4 newPerspectiveMatrix;
    double newDotDensity;
    std::size_t newNumDots;
    if (!computeNumDots(current_sizex,
                        current_sizey,
                        newZNear,
                        newZFar,
                        newFieldRadius,
                        newPerspectiveMatrix,
                        newDotDensity,
                        newNumDots))
    {
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
    // Dot size
    //
    
    auto newDotSize = dotSize->getValue().getFloat();
    if (newDotSize <= 0.0) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "Dot size must be greater than zero");
        return false;
    }
    
    //
    // Direction altitude
    //
    
    auto newDirectionAltitude = directionAltitude->getValue().getFloat();
    if (newDirectionAltitude < altitudeMin || newDirectionAltitude > altitudeMax) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "Direction altitude must be between %g and %g", altitudeMin, altitudeMax);
        return false;
    }
    
    //
    // Direction azimuth
    //
    
    auto newDirectionAzimuth = directionAzimuth->getValue().getFloat();
    if (newDirectionAzimuth < azimuthMin || newDirectionAzimuth > azimuthMax) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "Direction azimuth must be between %g and %g", azimuthMin, azimuthMax);
        return false;
    }
    
    //
    // Speed
    //
    
    auto newSpeed = speed->getValue().getFloat();
    if (newSpeed < 0.0) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "Speed must be non-negative");
        return false;
    }
    
    //
    // Coherence
    //
    
    auto newCoherence = coherence->getValue().getFloat();
    if ((newCoherence < 0.0) || (newCoherence > 1.0)) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "Coherence must be between 0 and 1");
        return false;
    }
    
    //
    // Lifetime
    //
    
    auto newLifetime = lifetime->getValue().getFloat();
    if (newLifetime < 0.0) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "Lifetime must be non-negative");
        return false;
    }
    
    previousSpeed = currentSpeed;
    previousCoherence = currentCoherence;
    previousLifetime = currentLifetime;
    previousFieldRadius = currentFieldRadius;
    previousNumDots = currentNumDots;
    
    currentZNear = newZNear;
    currentZFar = newZFar;
    currentDotDensity = newDotDensity;
    currentDotSize = newDotSize;
    currentDirectionAltitude = newDirectionAltitude;
    currentDirectionAzimuth = newDirectionAzimuth;
    currentSpeed = newSpeed;
    currentCoherence = newCoherence;
    currentLifetime = newLifetime;
    currentFieldRadius = newFieldRadius;
    currentPerspectiveMatrix = newPerspectiveMatrix;
    currentNumDots = newNumDots;
    
    return true;
}


void OpticFlowField::updateDots() {
    //
    // Update existing dots
    //
    
    const auto numValidDots = std::min(previousNumDots, currentNumDots);
    const auto dt = float(currentTime - previousTime) / 1.0e6f;
    const auto dr = dt * previousSpeed / previousFieldRadius;
    
    for (std::size_t i = 0; i < numValidDots; i++) {
        updateDot(i, dt, dr);
    }
    
    //
    // Add new dots (if needed)
    //
    
    for (auto i = previousNumDots; i < currentNumDots; i++) {
        replaceDot(i, newAge());
    }
}


void OpticFlowField::updateDot(std::size_t i, float dt, float dr) {
    auto &pos = dotPositions[i];
    auto &dir = dotDirections[i];
    auto &age = dotAges[i];
    
    if (previousLifetime != 0.0f) {
        age += dt;
        if (age > previousLifetime) {
            // The dot expired.  Replace it with a fresh one.
            replaceDot(i, 0.0f);
            return;
        }
    }
    
    pos.xyz += rotateVector(simd::make_float3(0.0f, 0.0f, dr),
                            dir.altitude,
                            dir.azimuth);
    if (simd::length_squared(pos.xyz) > 1.0f) {
        // The dot moved out of the field.  Move in a new one at the boundary.
        dir = newDirection();
        pos = rotateVector(simd::make_float4(0.0f, 0.0f, -1.0f + rand(0.0f, dr), 1.0f),
                           // A dot moving in the selected direction could enter the field at
                           // any point on the hemisphere centered at the direction vector, so
                           // we add a random offset of +/-90 degrees to the direction's
                           // altitude and azimuth to determine the dot's position
                           dir.altitude + rand(-90.0f, 90.0f),
                           dir.azimuth + rand(-90.0f, 90.0f));
        age = newAge();
        return;
    }
    
    if (currentCoherence != previousCoherence) {
        dir = newDirection();
    }
    
    if (currentLifetime != previousLifetime) {
        age = newAge();
    }
}


void OpticFlowField::replaceDot(std::size_t i, float age) {
    auto &pos = dotPositions[i];
    do {
        pos = simd::make_float4(rand(-1.0f, 1.0f),
                                rand(-1.0f, 1.0f),
                                rand(-1.0f, 1.0f),
                                1.0f);
    } while (simd::length_squared(pos.xyz) > 1.0f);
    
    dotDirections[i] = newDirection();
    dotAges[i] = age;
}


END_NAMESPACE_MW
