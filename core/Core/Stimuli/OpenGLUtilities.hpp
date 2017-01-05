//
//  OpenGLUtilities.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 1/5/17.
//
//

#ifndef OpenGLUtilities_hpp
#define OpenGLUtilities_hpp

#include <OpenGL/gl3.h>

#include <boost/noncopyable.hpp>

#include "Exceptions.h"


BEGIN_NAMESPACE_MW


class OpenGLException : public SimpleException {
    
public:
    explicit OpenGLException(const std::string &message, GLenum error = GL_NO_ERROR);
    
private:
    static std::string formatMessage(std::string message, GLenum error);
    
};


BEGIN_NAMESPACE(gl)


template <void (*deleter)(GLuint)>
class Object : boost::noncopyable {
    
public:
    explicit Object(GLuint obj) noexcept : obj(obj) { }
    ~Object() { if (obj) deleter(obj); }
    
    GLuint get() const noexcept { return obj; }
    
    GLuint release() noexcept {
        GLuint _obj = obj;
        obj = 0;
        return _obj;
    }
    
    explicit operator bool() const noexcept {
        return obj;
    }
    
private:
    GLuint obj;
    
};


using Shader = Object<glDeleteShader>;
using Program = Object<glDeleteProgram>;


GLuint createShader(GLenum shaderType, const std::string &shaderSource);
GLuint createProgram(const std::vector<GLuint> &shaders);


END_NAMESPACE(gl)


END_NAMESPACE_MW


#endif /* OpenGLUtilities_hpp */



























