//
//  OpenGLUtilities.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 1/5/17.
//
//

#ifndef OpenGLUtilities_hpp
#define OpenGLUtilities_hpp

#include <TargetConditionals.h>
#if TARGET_OS_OSX
#  define GL_SILENCE_DEPRECATION
#  define MWORKS_HAVE_OPENGL 1
#endif

#include <unordered_set>

#include <GLKit/GLKMath.h>


#if MWORKS_HAVE_OPENGL


#if TARGET_OS_OSX
#  include <OpenGL/gl3.h>
#else
#  error Unsupported platform
#endif

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
class Object {
    
public:
    ~Object() {
        if (obj) deleter(obj);
    }
    
    explicit Object(GLuint obj = 0) noexcept :
        obj(obj)
    { }
    
    // No copying
    Object(const Object &other) = delete;
    Object& operator=(const Object &other) = delete;
    
    // Move constructor
    Object(Object &&other) noexcept :
        obj(other.obj)
    {
        other.obj = 0;
    }
    
    // Move assignment
    Object& operator=(Object &&other) noexcept {
        std::swap(obj, other.obj);
        return (*this);
    }
    
    GLuint get() const noexcept {
        return obj;
    }
    
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


Shader createShader(GLenum shaderType, const std::string &shaderSource);
Shader createShader(GLenum shaderType, const std::string &glslVersion, const std::string &shaderSource);
Program createProgram(const std::vector<GLuint> &shaders);


template <GLenum target>
struct BufferBinding : boost::noncopyable {
    
    explicit BufferBinding(GLuint buffer) { bind(buffer); }
    ~BufferBinding() { bind(0); }
    
    void bind(GLuint buffer) { glBindBuffer(target, buffer); }
    
};


template <GLenum cap>
struct Enabled : boost::noncopyable {
    
    Enabled() { glEnable(cap); }
    ~Enabled() { glDisable(cap); }
    
};


template <GLenum target>
struct FramebufferBinding : boost::noncopyable {
    
    explicit FramebufferBinding(GLuint framebuffer) { bind(framebuffer); }
    ~FramebufferBinding() { bind(0); }
    
    void bind(GLuint framebuffer) { glBindFramebuffer(target, framebuffer); }
    
};


struct ProgramUsage : boost::noncopyable {
    
    explicit ProgramUsage(GLuint program) { use(program); }
    ~ProgramUsage() { use(0); }
    
    void use(GLuint program) { glUseProgram(program); }
    
};


template <GLenum target>
struct RenderbufferBinding : boost::noncopyable {
    
    explicit RenderbufferBinding(GLuint renderbuffer) { bind(renderbuffer); }
    ~RenderbufferBinding() { bind(0); }
    
    void bind(GLuint renderbuffer) { glBindRenderbuffer(target, renderbuffer); }
    
};


template <GLenum target>
struct TextureBinding : boost::noncopyable {
    
    explicit TextureBinding(GLuint texture) { bind(texture); }
    ~TextureBinding() { bind(0); }
    
    void bind(GLuint texture) { glBindTexture(target, texture); }
    
};


struct VertexArrayBinding : boost::noncopyable {
    
    explicit VertexArrayBinding(GLuint array) { bind(array); }
    ~VertexArrayBinding() { bind(0); }
    
    void bind(GLuint array) { glBindVertexArray(array); }
    
};


void getCurrentViewportSize(GLint &width, GLint &height);
std::unordered_set<std::string> getSupportedExtensions();
void logErrors();
void resetPixelStorageUnpackParameters(GLint alignment = 4);


END_NAMESPACE(gl)


END_NAMESPACE_MW


#endif // MWORKS_HAVE_OPENGL


#endif /* OpenGLUtilities_hpp */
