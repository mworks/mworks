/*
 *  MovingDots.cpp
 *  MovingDots
 *
 *  Created by Christopher Stawarz on 8/6/10.
 *  Copyright 2010 MIT. All rights reserved.
 *
 */

#include "MovingDots.h"


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
const std::string MovingDots::ANNOUNCE_DOTS("announce_dots");


void MovingDots::describeComponent(ComponentInfo &info) {
    StandardDynamicStimulus::describeComponent(info);
    
    info.setSignature("stimulus/moving_dots");
    info.setDisplayName("Moving Dots");
    info.setDescription("A moving dots stimulus.");

    info.addParameter(FIELD_RADIUS);
    info.addParameter(FIELD_CENTER_X);
    info.addParameter(FIELD_CENTER_Y);
    info.addParameter(DOT_DENSITY);
    info.addParameter(DOT_SIZE);
    info.addParameter(COLOR, "1.0,1.0,1.0");
    info.addParameter(ALPHA_MULTIPLIER, "1.0");
    info.addParameter(DIRECTION);
    info.addParameter(SPEED);
    info.addParameter(COHERENCE, "1.0");
    info.addParameter(LIFETIME, "0.0");
    info.addParameter(ANNOUNCE_DOTS, "0");
}


MovingDots::MovingDots(const ParameterValueMap &parameters) :
    StandardDynamicStimulus(parameters),
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
    announceDots(parameters[ANNOUNCE_DOTS]),
    previousFieldRadius(1.0f),
    currentFieldRadius(1.0f),
    previousNumDots(0),
    currentNumDots(0),
    previousSpeed(0.0f),
    currentSpeed(0.0f),
    previousCoherence(1.0f),
    currentCoherence(1.0f),
    previousLifetime(0.0f),
    currentLifetime(0.0f),
    dotSizeToPixels(0.0f),
    previousTime(-1),
    currentTime(-1)
{
    ParsedColorTrio color(parameters[COLOR]);
    red = registerVariable(color.getR());
    green = registerVariable(color.getG());
    blue = registerVariable(color.getB());
}


void MovingDots::load(shared_ptr<StimulusDisplay> display) {
    if (loaded)
        return;
    
    auto ctxLock = display->setCurrent(0);
    
    GLdouble xMin, xMax, yMin, yMax;
    display->getDisplayBounds(xMin, xMax, yMin, yMax);
    GLint width, height;
    display->getCurrentViewportSize(width, height);
    dotSizeToPixels = GLdouble(width) / (xMax - xMin);
    
    auto vertexShader = gl::createShader(GL_VERTEX_SHADER, vertexShaderSource);
    auto fragmentShader = gl::createShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    program = gl::createProgram({ vertexShader.get(), fragmentShader.get() }).release();
    gl::ProgramUsage programUsage(program);
    
    mvpMatrixUniformLocation = glGetUniformLocation(program, "mvpMatrix");
    pointSizeUniformLocation = glGetUniformLocation(program, "pointSize");
    colorUniformLocation = glGetUniformLocation(program, "color");
    
    glGenVertexArrays(1, &vertexArray);
    gl::VertexArrayBinding vertexArrayBinding(vertexArray);
    
    glGenBuffers(1, &dotPositionBuffer);
    gl::BufferBinding<GL_ARRAY_BUFFER> arrayBufferBinding(dotPositionBuffer);
    GLint dotPositionAttribLocation = glGetAttribLocation(program, "dotPosition");
    glEnableVertexAttribArray(dotPositionAttribLocation);
    glVertexAttribPointer(dotPositionAttribLocation, componentsPerDot, GL_FLOAT, GL_FALSE, 0, nullptr);
    
    StandardDynamicStimulus::load(display);
}


void MovingDots::unload(shared_ptr<StimulusDisplay> display) {
    if (!loaded)
        return;
    
    auto ctxLock = display->setCurrent(0);
    
    glDeleteBuffers(1, &dotPositionBuffer);
    glDeleteVertexArrays(1, &vertexArray);
    glDeleteProgram(program);
    
    StandardDynamicStimulus::unload(display);
}


Datum MovingDots::getCurrentAnnounceDrawData() {
    boost::mutex::scoped_lock locker(stim_lock);
    
    Datum announceData = StandardDynamicStimulus::getCurrentAnnounceDrawData();
    
    announceData.addElement(STIM_TYPE, "moving_dots");
    announceData.addElement(FIELD_RADIUS, currentFieldRadius);
    announceData.addElement(FIELD_CENTER_X, fieldCenterX->getValue().getFloat());
    announceData.addElement(FIELD_CENTER_Y, fieldCenterY->getValue().getFloat());
    announceData.addElement(DOT_DENSITY, dotDensity->getValue().getFloat());
    announceData.addElement(DOT_SIZE, dotSize->getValue().getFloat());
    announceData.addElement(STIM_COLOR_R, red->getValue().getFloat());
    announceData.addElement(STIM_COLOR_G, green->getValue().getFloat());
    announceData.addElement(STIM_COLOR_B, blue->getValue().getFloat());
    announceData.addElement(ALPHA_MULTIPLIER, alpha->getValue().getFloat());
    announceData.addElement(DIRECTION, direction->getValue().getFloat());
    announceData.addElement(SPEED, currentSpeed);
    announceData.addElement(COHERENCE, currentCoherence);
    announceData.addElement(LIFETIME, currentLifetime);
    announceData.addElement("num_dots", long(currentNumDots));
    
    if (announceDots->getValue().getBool()) {
        Datum dotsData(reinterpret_cast<char *>(&(dotPositions[0])), dotPositions.size() * sizeof(GLfloat));
        announceData.addElement("dots", dotsData);
    }
    
    return announceData;
}


void MovingDots::updateParameters() {
    //
    // Field radius, dot density, and number of dots
    //
    
    const double newFieldRadius = fieldRadius->getValue().getFloat();
    const double newDotDensity = dotDensity->getValue().getFloat();
    const GLint newNumDots = GLint(boost::math::round(newDotDensity * (M_PI * newFieldRadius * newFieldRadius)));
    
    if (newFieldRadius <= 0.0) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "Dot field radius must be greater than 0");
    } else if (newDotDensity <= 0.0) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "Dot field density must be greater than 0");
    } else if (newNumDots < 1) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "Dot field radius and dot density yield 0 dots");
    } else {
        previousFieldRadius = currentFieldRadius;
        currentFieldRadius = newFieldRadius;
        
        previousNumDots = currentNumDots;
        currentNumDots = newNumDots;
    }
    
    //
    // Speed
    //
    
    const GLfloat newSpeed = speed->getValue().getFloat();
    if (newSpeed < 0.0f) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "Dot field speed must be non-negative");
    } else {
        previousSpeed = currentSpeed;
        currentSpeed = newSpeed;
    }
    
    //
    // Coherence
    //
    
    const GLfloat newCoherence = coherence->getValue().getFloat();
    if ((newCoherence < 0.0f) || (newCoherence > 1.0f)) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "Dot field coherence must be between 0 and 1");
    } else {
        previousCoherence = currentCoherence;
        currentCoherence = newCoherence;
    }
    
    //
    // Lifetime
    //
    
    const GLfloat newLifetime = lifetime->getValue().getFloat();
    if (newLifetime < 0.0f) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "Dot field lifetime must be non-negative");
    } else {
        previousLifetime = currentLifetime;
        currentLifetime = newLifetime;
    }
}


void MovingDots::updateDots() {
    //
    // Update positions
    //
    
    const GLint numValidDots = std::min(previousNumDots, currentNumDots);
    const GLfloat dt = GLfloat(currentTime - previousTime) / 1.0e6f;
    const GLfloat dr = dt * previousSpeed / previousFieldRadius;
    
    for (GLint i = 0; i < numValidDots; i++) {
        GLfloat &age = getAge(i);
        age += dt;
        
        if ((age <= previousLifetime) || (previousLifetime == 0.0f)) {
            advanceDot(i, dt, dr);
        } else {
            replaceDot(i, newDirection(previousCoherence), 0.0f);
        }
    }
    
    //
    // Update directions
    //
    
    if (currentCoherence != previousCoherence) {
        for (GLint i = 0; i < numValidDots; i++) {
            getDirection(i) = newDirection(currentCoherence);
        }
    }
    
    //
    // Update lifetimes
    //
    
    if (currentLifetime != previousLifetime) {
        for (GLint i = 0; i < numValidDots; i++) {
            getAge(i) = newAge(currentLifetime);
        }
    }
    
    //
    // Add/remove dots
    //
    
    if (currentNumDots != previousNumDots) {
        dotPositions.resize(currentNumDots * componentsPerDot);
        dotDirections.resize(currentNumDots);
        dotAges.resize(currentNumDots);
        
        for (GLint i = previousNumDots; i < currentNumDots; i++) {
            replaceDot(i, newDirection(currentCoherence), newAge(currentLifetime));
        }
    }
}


void MovingDots::advanceDot(GLint i, GLfloat dt, GLfloat dr) {
    GLfloat &x = getX(i);
    GLfloat &y = getY(i);
    GLfloat &theta = getDirection(i);
    
    x += dr * std::cos(theta);
    y += dr * std::sin(theta);
    
    if (x*x + y*y > 1.0f) {
        theta = newDirection(previousCoherence);
        
        GLfloat y1 = rand(-1.0f, 1.0f);
        GLfloat x1 = -std::sqrt(1.0f - y1*y1) + rand(0.0f, dr);
        
        x = x1*std::cos(theta) - y1*std::sin(theta);
        y = x1*std::sin(theta) + y1*std::cos(theta);
        
        getAge(i) = newAge(previousLifetime);
    }
}


void MovingDots::replaceDot(GLint i, GLfloat direction, GLfloat age) {
    GLfloat &x = getX(i);
    GLfloat &y = getY(i);
    
    do {
        x = rand(-1.0f, 1.0f);
        y = rand(-1.0f, 1.0f);
    } while (x*x + y*y > 1.0f);
    
    getDirection(i) = direction;
    getAge(i) = age;
}


void MovingDots::drawFrame(shared_ptr<StimulusDisplay> display) {
    //
    // Update dots
    //
    
    currentTime = getElapsedTime();
    if (previousTime != currentTime) {
        updateParameters();
        updateDots();
        previousTime = currentTime;
        
        if (currentNumDots > 0) {
            gl::BufferBinding<GL_ARRAY_BUFFER> arrayBufferBinding(dotPositionBuffer);
            glBufferData(GL_ARRAY_BUFFER,
                         dotPositions.size() * sizeof(decltype(dotPositions)::value_type),
                         dotPositions.data(),
                         GL_STREAM_DRAW);
        }
    }
    
    if (0 == currentNumDots) {
        // No dots, so nothing to draw
        return;
    }
    
    //
    // Draw the dots
    //
    
    gl::ProgramUsage programUsage(program);
    gl::VertexArrayBinding vertexArrayBinding(vertexArray);
    
    auto currentMVPMatrix = GLKMatrix4Translate(display->getProjectionMatrix(),
                                                fieldCenterX->getValue().getFloat(),
                                                fieldCenterY->getValue().getFloat(),
                                                0.0);
    currentMVPMatrix = GLKMatrix4Scale(currentMVPMatrix, currentFieldRadius, currentFieldRadius, 1.0);
    currentMVPMatrix = GLKMatrix4Rotate(currentMVPMatrix,
                                        GLKMathDegreesToRadians(direction->getValue().getFloat()),
                                        0.0,
                                        0.0,
                                        1.0);
    glUniformMatrix4fv(mvpMatrixUniformLocation, 1, GL_FALSE, currentMVPMatrix.m);
    
    gl::Enabled<GL_PROGRAM_POINT_SIZE> pointSizeEnabled;
    glUniform1f(pointSizeUniformLocation, dotSize->getValue().getFloat() * dotSizeToPixels);
    
    auto currentColor = GLKVector4Make(red->getValue().getFloat(),
                                       green->getValue().getFloat(),
                                       blue->getValue().getFloat(),
                                       alpha->getValue().getFloat());
    glUniform4fv(colorUniformLocation, 1, currentColor.v);
    
    gl::Enabled<GL_BLEND> blendEnabled;
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glDrawArrays(GL_POINTS, 0, currentNumDots);
}


void MovingDots::stopPlaying() {
    StandardDynamicStimulus::stopPlaying();
    previousTime = -1;
}


const std::string MovingDots::vertexShaderSource
(R"(
 uniform mat4 mvpMatrix;
 uniform float pointSize;
 in vec4 dotPosition;
 
 void main() {
     gl_Position = mvpMatrix * dotPosition;
     gl_PointSize = pointSize;
 }
 )");


const std::string MovingDots::fragmentShaderSource
(R"(
 const vec2 center = vec2(0.5, 0.5);
 const float radius = 0.5;
 
 uniform vec4 color;
 out vec4 fragColor;
 
 void main() {
     //
     // Make the dots round, not square
     //
     // For an explanation of the edge-smoothing technique used here, see either of the following:
     // https://rubendv.be/blog/opengl/drawing-antialiased-circles-in-opengl/
     // http://www.numb3r23.net/2015/08/17/using-fwidth-for-distance-based-anti-aliasing/
     //
     float dist = distance(gl_PointCoord, center);
     float delta = fwidth(dist);
     if (dist > radius) {
         discard;
     }
     float alpha = 1.0;
     if (delta < radius) {  // If delta were greater than radius, we might erase the dot entirely
         alpha -= smoothstep(radius - delta, radius, dist);
     }
     fragColor.rgb = color.rgb;
     fragColor.a = alpha * color.a;
 }
 )");


END_NAMESPACE_MW























