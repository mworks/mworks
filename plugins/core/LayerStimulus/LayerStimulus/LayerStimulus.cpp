//
//  LayerStimulus.cpp
//  LayerStimulus
//
//  Created by Christopher Stawarz on 6/19/18.
//  Copyright © 2018 The MWorks Project. All rights reserved.
//

#include "LayerStimulus.hpp"


BEGIN_NAMESPACE_MW


void LayerStimulus::describeComponent(ComponentInfo &info) {
    Stimulus::describeComponent(info);
    info.setSignature("stimulus/layer");
}


LayerStimulus::LayerStimulus(const ParameterValueMap &parameters) :
    Stimulus(parameters)
{ }


void LayerStimulus::addChild(std::map<std::string, std::string> parameters,
                                     ComponentRegistryPtr reg,
                                     boost::shared_ptr<Component> child)
{
    auto stim = boost::dynamic_pointer_cast<Stimulus>(child);
    if (!stim) {
        throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Child component must be a stimulus");
    }
    children.push_back(stim);
}


void LayerStimulus::freeze(bool shouldFreeze) {
    for (auto &child : children) {
        child->freeze(shouldFreeze);
    }
    Stimulus::freeze(shouldFreeze);
}


namespace {
    const std::string vertexShaderSource
    (R"(
     in vec4 vertexPosition;
     in vec2 texCoords;
     
     out vec2 varyingTexCoords;
     
     void main() {
         gl_Position = vertexPosition;
         varyingTexCoords = texCoords;
     }
     )");
    
    
    const std::string fragmentShaderSource
    (R"(
     uniform sampler2D framebufferTexture;
     
     in vec2 varyingTexCoords;
     
     out vec4 fragColor;
     
     void main() {
         fragColor = texture(framebufferTexture, varyingTexCoords);
     }
     )");
    
    
    constexpr GLint numVertices = 4;
    constexpr GLint componentsPerVertex = 2;
    
    
    const std::array<GLfloat, numVertices*componentsPerVertex> vertexPositions
    {
        -1.0f, -1.0f,
         1.0f, -1.0f,
        -1.0f,  1.0f,
         1.0f,  1.0f,
    };
    
    
    const std::array<GLfloat, numVertices*componentsPerVertex> texCoords
    {
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f,
    };
}


void LayerStimulus::load(boost::shared_ptr<StimulusDisplay> display) {
    // Always load children, even if our loaded flag is true, as they may have been
    // unloaded independently
    for (auto &child : children) {
        child->load(display);
    }
    
    if (loaded)
        return;
    
    auto ctxLock = display->setCurrent(0);
    
    // Create program
    {
        auto vertexShader = gl::createShader(GL_VERTEX_SHADER, vertexShaderSource);
        auto fragmentShader = gl::createShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
        program = gl::createProgram({ vertexShader.get(), fragmentShader.get() }).release();
        gl::ProgramUsage programUsage(program);
        
        glUniform1i(glGetUniformLocation(program, "framebufferTexture"), 0);
        
        glGenBuffers(1, &vertexPositionBuffer);
        glGenBuffers(1, &texCoordsBuffer);
        glGenVertexArrays(1, &vertexArray);
        
        gl::VertexArrayBinding vertexArrayBinding(vertexArray);
        
        gl::BufferBinding<GL_ARRAY_BUFFER> arrayBufferBinding(vertexPositionBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPositions), vertexPositions.data(), GL_STATIC_DRAW);
        GLint vertexPositionAttribLocation = glGetAttribLocation(program, "vertexPosition");
        glEnableVertexAttribArray(vertexPositionAttribLocation);
        glVertexAttribPointer(vertexPositionAttribLocation, componentsPerVertex, GL_FLOAT, GL_FALSE, 0, nullptr);
        
        arrayBufferBinding.bind(texCoordsBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords.data(), GL_STATIC_DRAW);
        GLint texCoordsAttribLocation = glGetAttribLocation(program, "texCoords");
        glEnableVertexAttribArray(texCoordsAttribLocation);
        glVertexAttribPointer(texCoordsAttribLocation, componentsPerVertex, GL_FLOAT, GL_FALSE, 0, nullptr);
    }
    
    // Create framebuffer texture and framebuffer
    {
        glGenTextures(1, &framebufferTexture);
        gl::TextureBinding<GL_TEXTURE_2D> textureBinding(framebufferTexture);
        
        GLint viewportWidth, viewportHeight;
        gl::getCurrentViewportSize(viewportWidth, viewportHeight);
        
        // Provide initial data for the framebuffer texture, in hopes that this will force the
        // driver and/or GPU to allocate memory for it now, at load time, rather than on first
        // use.  Empirically, this seems to resolve intermittent issues with display update
        // timing on iOS.
        std::vector<std::uint16_t> data(4 * viewportWidth * viewportHeight, 0);
        
        gl::resetPixelStorageUnpackParameters(alignof(decltype(data)::value_type));
        
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_RGBA16F,
                     viewportWidth,
                     viewportHeight,
                     0,
                     GL_RGBA,
                     GL_HALF_FLOAT,
                     data.data());
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        glGenFramebuffers(1, &framebuffer);
        gl::FramebufferBinding<GL_DRAW_FRAMEBUFFER> framebufferBinding(framebuffer);
        
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTexture, 0);
        
        if (GL_FRAMEBUFFER_COMPLETE != glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER)) {
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "OpenGL framebuffer setup failed");
        }
    }
    
    Stimulus::load(display);
}


void LayerStimulus::unload(boost::shared_ptr<StimulusDisplay> display) {
    // Always unload children, even if our loaded flag is false, as they may have been
    // loaded independently
    for (auto &child : children) {
        child->unload(display);
    }
    
    if (!loaded)
        return;
    
    auto ctxLock = display->setCurrent(0);
    
    glDeleteFramebuffers(1, &framebuffer);
    glDeleteTextures(1, &framebufferTexture);
    glDeleteVertexArrays(1, &vertexArray);
    glDeleteBuffers(1, &texCoordsBuffer);
    glDeleteBuffers(1, &vertexPositionBuffer);
    glDeleteProgram(program);
    
    Stimulus::unload(display);
}


bool LayerStimulus::needDraw(boost::shared_ptr<StimulusDisplay> display) {
    for (auto &child : children) {
        if (child->needDraw(display)) {
            return true;
        }
    }
    return false;
}


void LayerStimulus::draw(boost::shared_ptr<StimulusDisplay> display) {
    //
    // Render children to framebuffer texture
    //
    
    display->pushFramebuffer(framebuffer);
    
    // Make background transparent
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    
    for (auto &child : children) {
        if (child->isLoaded()) {
            display->setRenderingMode(child->getRenderingMode());
            child->draw(display);
        } else {
            merror(M_DISPLAY_MESSAGE_DOMAIN,
                   "Stimulus \"%s\" (child of stimulus \"%s\") is not loaded and will not be displayed",
                   child->getTag().c_str(),
                   getTag().c_str());
        }
    }
    
    display->popFramebuffer();
    
    //
    // Render framebuffer texture to display
    //
    
    gl::ProgramUsage programUsage(program);
    gl::VertexArrayBinding vertexArrayBinding(vertexArray);
    gl::TextureBinding<GL_TEXTURE_2D> textureBinding(framebufferTexture);
    
    gl::Enabled<GL_BLEND> blendEnabled;
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, numVertices);
}


Datum LayerStimulus::getCurrentAnnounceDrawData() {
    auto announceData = Stimulus::getCurrentAnnounceDrawData();
    
    announceData.addElement(STIM_TYPE, "layer");
    
    Datum::list_value_type childAnnounceData;
    for (auto &child : children) {
        if (child->isLoaded()) {
            childAnnounceData.push_back(child->getCurrentAnnounceDrawData());
        }
    }
    announceData.addElement("children", Datum(childAnnounceData));
    
    return announceData;
}


END_NAMESPACE_MW




















