//
//  AppleStimulusDisplay.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 12/14/20.
//

#include "AppleStimulusDisplay.hpp"

#include "MWKMetalView_Private.h"
#include "OpenGLUtilities.hpp"


BEGIN_NAMESPACE_MW


AppleStimulusDisplay::AppleStimulusDisplay(bool useColorManagement) :
    StimulusDisplay(useColorManagement),
    contextManager(boost::dynamic_pointer_cast<AppleOpenGLContextManager>(opengl_context_manager)),
    framebufferWidth(0),
    framebufferHeight(0)
{ }


AppleStimulusDisplay::~AppleStimulusDisplay() {
}


MWKOpenGLContext * AppleStimulusDisplay::getMainContext() const {
    if (-1 != main_context_id) {
        return contextManager->getContext(main_context_id);
    }
    return nil;
}


MWKOpenGLContext * AppleStimulusDisplay::getMirrorContext() const {
    if (-1 != mirror_context_id) {
        return contextManager->getContext(mirror_context_id);
    }
    return getMainContext();
}


MWKMetalView * AppleStimulusDisplay::getMainView() const {
    if (-1 != main_context_id) {
        return contextManager->getView(main_context_id);
    }
    return nil;
}


MWKMetalView * AppleStimulusDisplay::getMirrorView() const {
    if (-1 != mirror_context_id) {
        return contextManager->getView(mirror_context_id);
    }
    return getMainView();
}


void AppleStimulusDisplay::prepareContext(int context_id) {
    @autoreleasepool {
        StimulusDisplay::prepareContext(context_id);
        
        if (context_id == main_context_id) {
            MWKMetalView *view = contextManager->getView(context_id);
            MWKOpenGLContext *context = contextManager->getContext(context_id);
            
            framebufferWidth = view.drawableSize.width;
            framebufferHeight = view.drawableSize.height;
            
            {
                NSDictionary *pixelBufferAttributes = @{
                    (NSString *)kCVPixelBufferPixelFormatTypeKey: @(kCVPixelFormatType_64RGBAHalf),
                    (NSString *)kCVPixelBufferWidthKey: @(framebufferWidth),
                    (NSString *)kCVPixelBufferHeightKey: @(framebufferHeight),
                    (NSString *)kCVPixelBufferMetalCompatibilityKey: @(YES),
#if TARGET_OS_OSX
                    (NSString *)kCVPixelBufferOpenGLCompatibilityKey: @(YES),
#else
                    (NSString *)kCVPixelBufferOpenGLESCompatibilityKey: @(YES),
#endif
                };
                CVPixelBufferPoolRef _cvPixelBufferPool = nullptr;
                auto status = CVPixelBufferPoolCreate(kCFAllocatorDefault,
                                                      nullptr,
                                                      (__bridge CFDictionaryRef)pixelBufferAttributes,
                                                      &_cvPixelBufferPool);
                if (status != kCVReturnSuccess) {
                    throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN,
                                          boost::format("Cannot create pixel buffer pool (error = %d)") % status);
                }
                cvPixelBufferPool = CVPixelBufferPoolPtr::owned(_cvPixelBufferPool);
            }
            
            {
                CVMetalTextureCacheRef _cvMetalTextureCache = nullptr;
                auto status = CVMetalTextureCacheCreate(kCFAllocatorDefault,
                                                        nil,
                                                        view.device,
                                                        nil,
                                                        &_cvMetalTextureCache);
                if (status != kCVReturnSuccess) {
                    throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN,
                                          boost::format("Cannot create Metal texture cache (error = %d)") % status);
                }
                cvMetalTextureCache = CVMetalTextureCachePtr::created(_cvMetalTextureCache);
            }
            
            {
#if TARGET_OS_OSX
                CVOpenGLTextureCacheRef _cvOpenGLTextureCache = nullptr;
                auto status = CVOpenGLTextureCacheCreate(kCFAllocatorDefault,
                                                         nil,
                                                         context.CGLContextObj,
                                                         context.pixelFormat.CGLPixelFormatObj,
                                                         nil,
                                                         &_cvOpenGLTextureCache);
#else
                CVOpenGLESTextureCacheRef _cvOpenGLTextureCache = nullptr;
                auto status = CVOpenGLESTextureCacheCreate(kCFAllocatorDefault,
                                                           nil,
                                                           context,
                                                           nil,
                                                           &_cvOpenGLTextureCache);
#endif
                if (status != kCVReturnSuccess) {
                    throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN,
                                          boost::format("Cannot create OpenGL texture cache (error = %d)") % status);
                }
                cvOpenGLTextureCache = CVOpenGLTextureCachePtr::created(_cvOpenGLTextureCache);
            }
        }
    }
}


int AppleStimulusDisplay::createFramebuffer() {
    Framebuffer framebuffer;
    
    {
        CVPixelBufferRef _cvPixelBuffer = nullptr;
        auto status = CVPixelBufferPoolCreatePixelBuffer(kCFAllocatorDefault,
                                                         cvPixelBufferPool.get(),
                                                         &_cvPixelBuffer);
        if (status != kCVReturnSuccess) {
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN,
                                  boost::format("Cannot create pixel buffer (error = %d)") % status);
        }
        framebuffer.cvPixelBuffer = CVPixelBufferPtr::owned(_cvPixelBuffer);
    }
    
    {
        CVMetalTextureRef _cvMetalTexture = nullptr;
        auto status = CVMetalTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
                                                                cvMetalTextureCache.get(),
                                                                framebuffer.cvPixelBuffer.get(),
                                                                nil,
                                                                MTLPixelFormatRGBA16Float,
                                                                framebufferWidth,
                                                                framebufferHeight,
                                                                0,
                                                                &_cvMetalTexture);
        if (status != kCVReturnSuccess) {
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN,
                                  boost::format("Cannot create Metal texture (error = %d)") % status);
        }
        framebuffer.cvMetalTexture = CVMetalTexturePtr::created(_cvMetalTexture);
    }
    
    {
#if TARGET_OS_OSX
        CVOpenGLTextureRef _cvOpenGLTexture = nullptr;
        auto status = CVOpenGLTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
                                                                 cvOpenGLTextureCache.get(),
                                                                 framebuffer.cvPixelBuffer.get(),
                                                                 nil,
                                                                 &_cvOpenGLTexture);
#else
        CVOpenGLESTextureRef _cvOpenGLTexture = nullptr;
        auto status = CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
                                                                   cvOpenGLTextureCache.get(),
                                                                   framebuffer.cvPixelBuffer.get(),
                                                                   nil,
                                                                   GL_TEXTURE_2D,
                                                                   GL_RGBA16F,
                                                                   framebufferWidth,
                                                                   framebufferHeight,
                                                                   GL_RGBA,
                                                                   GL_HALF_FLOAT,
                                                                   0,
                                                                   &_cvOpenGLTexture);
#endif
        if (status != kCVReturnSuccess) {
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN,
                                  boost::format("Cannot create OpenGL texture (error = %d)") % status);
        }
        framebuffer.cvOpenGLTexture = CVOpenGLTexturePtr::created(_cvOpenGLTexture);
    }
    
    {
        GLuint _glFramebuffer = 0;
        glGenFramebuffers(1, &_glFramebuffer);
        gl::FramebufferBinding<GL_DRAW_FRAMEBUFFER> framebufferBinding(_glFramebuffer);
        
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER,
                               GL_COLOR_ATTACHMENT0,
#if TARGET_OS_OSX
                               CVOpenGLTextureGetTarget(framebuffer.cvOpenGLTexture.get()),
                               CVOpenGLTextureGetName(framebuffer.cvOpenGLTexture.get()),
#else
                               CVOpenGLESTextureGetTarget(framebuffer.cvOpenGLTexture.get()),
                               CVOpenGLESTextureGetName(framebuffer.cvOpenGLTexture.get()),
#endif
                               0);
        
        if (GL_FRAMEBUFFER_COMPLETE != glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER)) {
            throw SimpleException("OpenGL framebuffer setup failed");
        }
        
        framebuffer.glFramebuffer = _glFramebuffer;
    }
    
    int framebuffer_id = 0;
    auto lastIter = framebuffers.rbegin();
    if (lastIter != framebuffers.rend()) {
        framebuffer_id = lastIter->first + 1;
    }
    framebuffers.emplace(framebuffer_id, std::move(framebuffer));
    return framebuffer_id;
}


void AppleStimulusDisplay::pushFramebuffer(int framebuffer_id) {
    auto iter = framebuffers.find(framebuffer_id);
    if (iter == framebuffers.end()) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "Internal error: invalid framebuffer ID: %d", framebuffer_id);
        return;
    }
    framebufferStack.emplace_back(iter->second);
    bindCurrentFramebuffer();
}


void AppleStimulusDisplay::bindCurrentFramebuffer() {
    if (framebufferStack.empty()) {
        // If the framebuffer stack is empty, then we're about to draw to the screen.
        // Bind the default framebuffer, then call glFlush to ensure that any pending
        // OpenGL commands are committed before we proceed.
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glFlush();
        return;
    }
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebufferStack.back().glFramebuffer);
    const GLenum drawBuffer = GL_COLOR_ATTACHMENT0;
    glDrawBuffers(1, &drawBuffer);
    glViewport(0, 0, framebufferWidth, framebufferHeight);
}


void AppleStimulusDisplay::presentFramebuffer(int framebuffer_id, int dst_context_id) {
    @autoreleasepool {
        auto iter = framebuffers.find(framebuffer_id);
        if (iter == framebuffers.end()) {
            merror(M_DISPLAY_MESSAGE_DOMAIN, "Internal error: invalid framebuffer ID: %d", framebuffer_id);
            return;
        }
        if (auto dstView = contextManager->getView(dst_context_id)) {
            [dstView drawTexture:CVMetalTextureGetTexture(iter->second.cvMetalTexture.get())];
        }
    }
}


id<MTLTexture> AppleStimulusDisplay::getCurrentMetalFramebufferTexture() const {
    if (framebufferStack.empty()) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "Internal error: no current Metal framebuffer texture");
        return nil;
    }
    return CVMetalTextureGetTexture(framebufferStack.back().cvMetalTexture.get());
}


void AppleStimulusDisplay::popFramebuffer() {
    if (framebufferStack.empty()) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "Internal error: no framebuffer to pop");
        return;
    }
    framebufferStack.pop_back();
    bindCurrentFramebuffer();
}


void AppleStimulusDisplay::releaseFramebuffer(int framebuffer_id) {
    auto iter = framebuffers.find(framebuffer_id);
    if (iter == framebuffers.end()) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "Internal error: invalid framebuffer ID: %d", framebuffer_id);
        return;
    }
    framebuffers.erase(iter);
}


END_NAMESPACE_MW
