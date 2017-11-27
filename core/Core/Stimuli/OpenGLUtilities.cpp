//
//  OpenGLUtilities.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 1/5/17.
//
//

#include "OpenGLUtilities.hpp"

#include "Utilities.h"


BEGIN_NAMESPACE_MW


OpenGLException::OpenGLException(const std::string &message, GLenum error) :
    SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "OpenGL", formatMessage(message, error))
{ }


std::string OpenGLException::formatMessage(std::string message, GLenum error) {
    if (error != GL_NO_ERROR) {
        message += (boost::format(" (0x%04X)") % error).str();
    }
    return message;
}


BEGIN_NAMESPACE(gl)


namespace {
    const std::string defaultGLSLVersion
    (
#if MWORKS_OPENGL_ES
     "300 es"
#else
     "330"
#endif
    );
}


Shader createShader(GLenum shaderType, const std::string &shaderSource) {
    return createShader(shaderType, defaultGLSLVersion, shaderSource);
}


Shader createShader(GLenum shaderType, const std::string &glslVersion, const std::string &shaderSource) {
    Shader shader(glCreateShader(shaderType));
    if (!shader) {
        throw OpenGLException("Shader creation failed", glGetError());
    }
    
    const std::array<const GLchar *, 5> strings = {
        "#version ", glslVersion.data(), "\n",
        "precision highp float;\n",
        shaderSource.data()
    };
    glShaderSource(shader.get(), strings.size(), strings.data(), nullptr);
    glCompileShader(shader.get());
    
    GLint infoLogLength;
    glGetShaderiv(shader.get(), GL_INFO_LOG_LENGTH, &infoLogLength);
    if (infoLogLength > 0) {
        std::vector<GLchar> infoLog(infoLogLength);
        glGetShaderInfoLog(shader.get(), infoLogLength, nullptr, infoLog.data());
        mwarning(M_DISPLAY_MESSAGE_DOMAIN,
                 "OpenGL: Shader compilation produced the following information log:\n%s",
                 infoLog.data());
    }
    
    GLint compileStatus;
    glGetShaderiv(shader.get(), GL_COMPILE_STATUS, &compileStatus);
    if (compileStatus != GL_TRUE) {
        throw OpenGLException("Shader compilation failed");
    }
    
    return shader;
}


Program createProgram(const std::vector<GLuint> &shaders) {
    Program program(glCreateProgram());
    if (!program) {
        throw OpenGLException("Program creation failed", glGetError());
    }
    
    for (GLuint shader : shaders) {
        glAttachShader(program.get(), shader);
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            throw OpenGLException("Shader attachment failed", error);
        }
    }
    
    glLinkProgram(program.get());
    
    GLint infoLogLength;
    glGetProgramiv(program.get(), GL_INFO_LOG_LENGTH, &infoLogLength);
    if (infoLogLength > 0) {
        std::vector<GLchar> infoLog(infoLogLength);
        glGetProgramInfoLog(program.get(), infoLogLength, nullptr, infoLog.data());
        mwarning(M_DISPLAY_MESSAGE_DOMAIN,
                 "OpenGL: Program linking produced the following information log:\n%s",
                 infoLog.data());
    }
    
    GLint linkStatus;
    glGetProgramiv(program.get(), GL_LINK_STATUS, &linkStatus);
    if (linkStatus != GL_TRUE) {
        throw OpenGLException("Program linking failed");
    }
    
    return program;
}


std::unordered_set<std::string> getSupportedExtensions() {
    std::unordered_set<std::string> supportedExtensions;
    
    GLint numExtensions;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
    for (GLuint i = 0; i < numExtensions; i++) {
        supportedExtensions.emplace(reinterpret_cast<const char *>(glGetStringi(GL_EXTENSIONS, i)));
    }
    
    return supportedExtensions;
}


void logErrors() {
    GLenum error;
    while (GL_NO_ERROR != (error = glGetError())) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "GL error = 0x%04X", error);
    }
}


void resetPixelStorageUnpackParameters(GLint alignment) {
    //
    // Set all pixel unpacking parameters to their default values (possibly excluding GL_UNPACK_ALIGNMENT)
    //
#if !MWORKS_OPENGL_ES
    glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
    glPixelStorei(GL_UNPACK_LSB_FIRST, GL_FALSE);
#endif
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_IMAGES, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
}


END_NAMESPACE(gl)


END_NAMESPACE_MW



























