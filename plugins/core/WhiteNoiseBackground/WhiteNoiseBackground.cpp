/*
 *  WhiteNoiseBackground.cpp
 *  WhiteNoiseBackground
 *
 *  Created by Christopher Stawarz on 12/15/10.
 *  Copyright 2010 MIT. All rights reserved.
 *
 */

#include "WhiteNoiseBackground.h"


BEGIN_NAMESPACE_MW


void WhiteNoiseBackground::describeComponent(ComponentInfo &info) {
    Stimulus::describeComponent(info);
    info.setSignature("stimulus/white_noise_background");
}


WhiteNoiseBackground::WhiteNoiseBackground(const ParameterValueMap &parameters) :
    Stimulus(parameters),
    randGen(Clock::instance()->getSystemTimeUS()),
    randDist(std::numeric_limits<GLubyte>::min(), std::numeric_limits<GLubyte>::max())
{ }


void WhiteNoiseBackground::load(shared_ptr<StimulusDisplay> display) {
    if (loaded)
        return;
    
    auto ctxLock = display->setCurrent(0);
    
    display->getCurrentViewportSize(width, height);
    pixels.clear();
    pixels.assign(width * height, std::numeric_limits<GLuint>::max());
    
    auto vertexShader = gl::createShader(GL_VERTEX_SHADER, vertexShaderSource);
    auto fragmentShader = gl::createShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    program = gl::createProgram({ vertexShader.get(), fragmentShader.get() }).release();
    gl::ProgramUsage programUsage(program);
    
    glUniform1i(glGetUniformLocation(program, "noiseTexture"), 0);
    
    glGenVertexArrays(1, &vertexArray);
    gl::VertexArrayBinding vertexArrayBinding(vertexArray);
    
    glGenBuffers(1, &vertexPositionBuffer);
    gl::BufferBinding<GL_ARRAY_BUFFER> arrayBufferBinding(vertexPositionBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPositions), vertexPositions.data(), GL_STATIC_DRAW);
    GLint vertexPositionAttribLocation = glGetAttribLocation(program, "vertexPosition");
    glEnableVertexAttribArray(vertexPositionAttribLocation);
    glVertexAttribPointer(vertexPositionAttribLocation, componentsPerVertex, GL_FLOAT, GL_FALSE, 0, nullptr);
    
    glGenBuffers(1, &texCoordsBuffer);
    arrayBufferBinding.bind(texCoordsBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords.data(), GL_STATIC_DRAW);
    GLint texCoordsAttribLocation = glGetAttribLocation(program, "texCoords");
    glEnableVertexAttribArray(texCoordsAttribLocation);
    glVertexAttribPointer(texCoordsAttribLocation, componentsPerVertex, GL_FLOAT, GL_FALSE, 0, nullptr);
    
    glGenTextures(1, &texture);
    gl::TextureBinding<GL_TEXTURE_2D> textureBinding(texture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    
    Stimulus::load(display);
}


void WhiteNoiseBackground::unload(shared_ptr<StimulusDisplay> display) {
    if (!loaded)
        return;
    
    auto ctxLock = display->setCurrent(0);
    
    glDeleteTextures(1, &texture);
    glDeleteBuffers(1, &texCoordsBuffer);
    glDeleteBuffers(1, &vertexPositionBuffer);
    glDeleteVertexArrays(1, &vertexArray);
    glDeleteProgram(program);
    
    pixels.clear();
    
    Stimulus::unload(display);
}


void WhiteNoiseBackground::draw(shared_ptr<StimulusDisplay> display) {
    gl::ProgramUsage programUsage(program);
    gl::VertexArrayBinding vertexArrayBinding(vertexArray);
    
    gl::TextureBinding<GL_TEXTURE_2D> textureBinding(texture);
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, numVertices);
}


Datum WhiteNoiseBackground::getCurrentAnnounceDrawData() {
    Datum announceData = Stimulus::getCurrentAnnounceDrawData();
	announceData.addElement(STIM_TYPE, "white_noise_background");
    return announceData;
}


void WhiteNoiseBackground::randomizePixels() {
    boost::mutex::scoped_lock lock(pixelsMutex);

    for (size_t i = 0; i < pixels.size(); i++) {
        GLubyte randVal = randDist(randGen);
        GLubyte *pix = reinterpret_cast<GLubyte *>(&(pixels[i]));
        for (size_t j = 0; j < componentsPerPixel - 1; j++) {
            pix[j] = randVal;
        }
    }
    
    auto ctxLock = StimulusDisplay::getCurrentStimulusDisplay()->setCurrent(0);
    updateTexture();
}


void WhiteNoiseBackground::updateTexture() {
#if !MWORKS_OPENGL_ES
    glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
    glPixelStorei(GL_UNPACK_LSB_FIRST, GL_FALSE);
#endif
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_IMAGES, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    gl::TextureBinding<GL_TEXTURE_2D> textureBinding(texture);
    
    glTexSubImage2D(GL_TEXTURE_2D,
                    0,
                    0,
                    0,
                    width,
                    height,
#if MWORKS_OPENGL_ES
                    GL_RGBA,
                    GL_UNSIGNED_BYTE,
#else
                    GL_BGRA,
                    GL_UNSIGNED_INT_8_8_8_8_REV,
#endif
                    pixels.data());
}


const std::string WhiteNoiseBackground::vertexShaderSource
(R"(
 in vec4 vertexPosition;
 in vec2 texCoords;
 out vec2 varyingTexCoords;
 
 void main() {
     gl_Position = vertexPosition;
     varyingTexCoords = texCoords;
 }
 )");


const std::string WhiteNoiseBackground::fragmentShaderSource
(R"(
 uniform sampler2D noiseTexture;
 in vec2 varyingTexCoords;
 out vec4 fragColor;
 
 void main() {
     fragColor = texture(noiseTexture, varyingTexCoords);
 }
 )");


const WhiteNoiseBackground::VertexPositionArray WhiteNoiseBackground::vertexPositions {
    -1.0f, -1.0f,
     1.0f, -1.0f,
    -1.0f,  1.0f,
     1.0f,  1.0f
};


const WhiteNoiseBackground::VertexPositionArray WhiteNoiseBackground::texCoords {
    0.0f, 0.0f,
    1.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f
};


END_NAMESPACE_MW























