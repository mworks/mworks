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


const std::string WhiteNoiseBackground::GRAYSCALE("grayscale");
const std::string WhiteNoiseBackground::RAND_SEED("rand_seed");
const std::string WhiteNoiseBackground::RANDOMIZE_ON_DRAW("randomize_on_draw");


void WhiteNoiseBackground::describeComponent(ComponentInfo &info) {
    Stimulus::describeComponent(info);
    
    info.setSignature("stimulus/white_noise_background");
    
    info.addParameter(GRAYSCALE, "YES");
    info.addParameter(RAND_SEED, false);
    info.addParameter(RANDOMIZE_ON_DRAW, "NO");
}


WhiteNoiseBackground::WhiteNoiseBackground(const ParameterValueMap &parameters) :
    Stimulus(parameters),
    grayscale(parameters[GRAYSCALE]),
    numChannels(grayscale ? 1 : 3),
    randSeed(0),
    randCount(0),
    randomizeOnDraw(parameters[RANDOMIZE_ON_DRAW]),
    shouldRandomize(false),
    seedTextures(numChannels, 0),
    noiseTextures(numChannels, 0)
{
    if (parameters[RAND_SEED].empty()) {
        randSeed = Clock::instance()->getSystemTimeNS();
    } else {
        randSeed = MWTime(parameters[RAND_SEED]);
    }
}


void WhiteNoiseBackground::load(shared_ptr<StimulusDisplay> display) {
    if (loaded)
        return;
    
    auto ctxLock = display->setCurrent(0);
    
    display->getCurrentViewportSize(width, height);
    
    // Create vertex attribute buffers
    {
        glGenBuffers(1, &vertexPositionBuffer);
        gl::BufferBinding<GL_ARRAY_BUFFER> arrayBufferBinding(vertexPositionBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPositions), vertexPositions.data(), GL_STATIC_DRAW);
        
        glGenBuffers(1, &texCoordsBuffer);
        arrayBufferBinding.bind(texCoordsBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords.data(), GL_STATIC_DRAW);
    }
    
    // Create programs
    {
        auto vertexShader = gl::createShader(GL_VERTEX_SHADER, vertexShaderSource);
        
        {
            createProgram(noiseGenProgram, vertexShader, noiseGenFragmentShaderSource);
            gl::ProgramUsage programUsage(noiseGenProgram);
            glUniform1i(glGetUniformLocation(noiseGenProgram, "noiseTexture"), 0);
        }
        
        {
            createProgram(noiseRenderProgram, vertexShader, noiseRenderFragmentShaderSource);
            gl::ProgramUsage programUsage(noiseRenderProgram);
            glUniform1i(glGetUniformLocation(noiseRenderProgram, "redNoiseTexture"), 0);
            glUniform1i(glGetUniformLocation(noiseRenderProgram, "greenNoiseTexture"), 1);
            glUniform1i(glGetUniformLocation(noiseRenderProgram, "blueNoiseTexture"), 2);
            glUniform1i(glGetUniformLocation(noiseRenderProgram, "grayscale"), grayscale);
        }
    }
    
    // Create textures
    {
        std::vector<GLuint> data(width * height);
        std::mt19937 seedGen(randSeed);
        std::uniform_int_distribution<GLuint> seedDist(std::minstd_rand::min(), std::minstd_rand::max());
        for (int i = 0; i < numChannels; i++) {
            //
            // Generate a seed for each noise texel.
            //
            // Although we use a linear congruential generator (LCG) on the GPU to update the texels, we use the
            // Mersenne Twister to generate the per-texel seeds.  If we used the same generator in both places,
            // then texel n at iteration i+1 would have the same value as texel n+1 at iteration i.
            //
            for (auto &value : data) {
                value = seedDist(seedGen);
            }
            createTexture(seedTextures.at(i), data);
            createTexture(noiseTextures.at(i), data);
        }
    }
    
    Stimulus::load(display);
}


void WhiteNoiseBackground::unload(shared_ptr<StimulusDisplay> display) {
    if (!loaded)
        return;
    
    auto ctxLock = display->setCurrent(0);
    
    for (auto &item : framebuffers) {
        glDeleteFramebuffers(1, &(item.second));
    }
    glDeleteTextures(noiseTextures.size(), noiseTextures.data());
    glDeleteTextures(seedTextures.size(), seedTextures.data());
    for (auto &item : vertexArrays) {
        glDeleteVertexArrays(1, &(item.second));
    }
    glDeleteProgram(noiseRenderProgram);
    glDeleteProgram(noiseGenProgram);
    glDeleteBuffers(1, &texCoordsBuffer);
    glDeleteBuffers(1, &vertexPositionBuffer);
    
    Stimulus::unload(display);
}


void WhiteNoiseBackground::draw(shared_ptr<StimulusDisplay> display) {
    if (randomizeOnDraw || shouldRandomize.exchange(false)) {
        // Previous noise values become current seeds
        std::swap(seedTextures, noiseTextures);
        
        glUseProgram(noiseGenProgram);
        glBindVertexArray(vertexArrays.at(noiseGenProgram));
        
        for (int i = 0; i < numChannels; i++) {
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffers.at(noiseTextures.at(i)));
            const GLenum drawBuffer = GL_COLOR_ATTACHMENT0;
            glDrawBuffers(1, &drawBuffer);
            
            glBindTexture(GL_TEXTURE_2D, seedTextures.at(i));
            glDrawArrays(GL_TRIANGLE_STRIP, 0, numVertices);
        }
        
        display->bindDefaultFramebuffer(display->getCurrentContextIndex());
        randCount++;
    }
    
    gl::ProgramUsage programUsage(noiseRenderProgram);
    gl::VertexArrayBinding vertexArrayBinding(vertexArrays.at(noiseRenderProgram));
    
    glBindTexture(GL_TEXTURE_2D, noiseTextures.at(0));
    if (numChannels == 3) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, noiseTextures.at(1));
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, noiseTextures.at(2));
    }
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, numVertices);
    
    if (numChannels == 3) {
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE0);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}


Datum WhiteNoiseBackground::getCurrentAnnounceDrawData() {
    Datum announceData = Stimulus::getCurrentAnnounceDrawData();
    announceData.addElement(STIM_TYPE, "white_noise_background");
    announceData.addElement(GRAYSCALE, grayscale);
    announceData.addElement(RAND_SEED, randSeed);
    announceData.addElement("rand_count", static_cast<long long>(randCount));
    announceData.addElement(RANDOMIZE_ON_DRAW, randomizeOnDraw);
    return announceData;
}


void WhiteNoiseBackground::randomize() {
    shouldRandomize = true;
}


void WhiteNoiseBackground::createProgram(GLuint &program,
                                         const gl::Shader &vertexShader,
                                         const std::string &fragmentShaderSource)
{
    auto fragmentShader = gl::createShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    program = gl::createProgram({ vertexShader.get(), fragmentShader.get() }).release();
    
    auto &vertexArray = vertexArrays[program];
    glGenVertexArrays(1, &vertexArray);
    gl::VertexArrayBinding vertexArrayBinding(vertexArray);
    
    gl::BufferBinding<GL_ARRAY_BUFFER> arrayBufferBinding(vertexPositionBuffer);
    GLint vertexPositionAttribLocation = glGetAttribLocation(program, "vertexPosition");
    glEnableVertexAttribArray(vertexPositionAttribLocation);
    glVertexAttribPointer(vertexPositionAttribLocation, componentsPerVertex, GL_FLOAT, GL_FALSE, 0, nullptr);
    
    arrayBufferBinding.bind(texCoordsBuffer);
    GLint texCoordsAttribLocation = glGetAttribLocation(program, "texCoords");
    glEnableVertexAttribArray(texCoordsAttribLocation);
    glVertexAttribPointer(texCoordsAttribLocation, componentsPerVertex, GL_FLOAT, GL_FALSE, 0, nullptr);
}


void WhiteNoiseBackground::createTexture(GLuint &texture, const std::vector<GLuint> &data) {
    glGenTextures(1, &texture);
    gl::TextureBinding<GL_TEXTURE_2D> textureBinding(texture);
    
#if !MWORKS_OPENGL_ES
    glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
    glPixelStorei(GL_UNPACK_LSB_FIRST, GL_FALSE);
#endif
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_IMAGES, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, alignof(GLuint));
    
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_R32UI,
                 width,
                 height,
                 0,
                 GL_RED_INTEGER,
                 GL_UNSIGNED_INT,
                 data.data());
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    
    auto &framebuffer = framebuffers[texture];
    glGenFramebuffers(1, &framebuffer);
    gl::FramebufferBinding<GL_DRAW_FRAMEBUFFER> framebufferBinding(framebuffer);
    
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    
    if (GL_FRAMEBUFFER_COMPLETE != glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER)) {
        throw SimpleException("OpenGL framebuffer setup failed");
    }
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


const std::string WhiteNoiseBackground::noiseGenFragmentShaderSource
(R"(
 precision highp int;
 
 // LCG parameters used by std::minstd_rand
 const uint a = 48271u;
 const uint c = 0u;
 const uint m = 2147483647u;
 
 uniform highp usampler2D noiseTexture;
 
 in vec2 varyingTexCoords;
 out uint noiseVal;
 
 void main() {
     uint lastNoiseVal = texture(noiseTexture, varyingTexCoords).r;
     noiseVal = (a * lastNoiseVal + c) % m;
 }
 )");


const std::string WhiteNoiseBackground::noiseRenderFragmentShaderSource
(R"(
 const float noiseMax = 2147483646.0;  // m-1
 
 uniform highp usampler2D redNoiseTexture;
 uniform highp usampler2D greenNoiseTexture;
 uniform highp usampler2D blueNoiseTexture;
 uniform bool grayscale;
 
 in vec2 varyingTexCoords;
 out vec4 fragColor;
 
 float normNoise(highp usampler2D noiseTexture) {
     return float(texture(noiseTexture, varyingTexCoords).r) / noiseMax;
 }
 
 void main() {
     if (grayscale) {
         float noiseVal = normNoise(redNoiseTexture);
         fragColor = vec4(noiseVal, noiseVal, noiseVal, 1.0);
     } else {
         fragColor = vec4(normNoise(redNoiseTexture),
                          normNoise(greenNoiseTexture),
                          normNoise(blueNoiseTexture),
                          1.0);
     }
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























