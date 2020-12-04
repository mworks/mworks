//
//  AppleOpenGLContextManager.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 3/27/17.
//
//

#ifndef AppleOpenGLContextManager_hpp
#define AppleOpenGLContextManager_hpp

#include <TargetConditionals.h>
#if TARGET_OS_OSX
#  define GL_SILENCE_DEPRECATION
#  include <Cocoa/Cocoa.h>
#elif TARGET_OS_IPHONE
#  define GLES_SILENCE_DEPRECATION
#  include <UIKit/UIKit.h>
#  include <OpenGLES/EAGL.h>
#endif

#include <MetalKit/MetalKit.h>

#include "OpenGLContextManager.h"
#include "CFObjectPtr.h"


#if TARGET_OS_OSX
@interface MWKOpenGLContext : NSOpenGLContext
#elif TARGET_OS_IPHONE
@interface MWKOpenGLContext : EAGLContext
#else
#error Unsupported platform
#endif

- (mw::OpenGLContextLock)lockContext;

@end


@interface MWKMetalView : MTKView

@property(nonatomic, readonly) id<MTLCommandQueue> commandQueue;

- (BOOL)prepareUsingColorManagement:(BOOL)useColorManagement error:(NSError **)error;

@end


BEGIN_NAMESPACE_MW


class AppleOpenGLContextManager : public OpenGLContextManager {
    
public:
    AppleOpenGLContextManager();
    ~AppleOpenGLContextManager();
    
    MWKOpenGLContext * getContext(int context_id) const;
    MWKOpenGLContext * getFullscreenContext() const;
    MWKOpenGLContext * getMirrorContext() const;
    
    MWKMetalView * getView(int context_id) const;
    MWKMetalView * getFullscreenView() const;
    MWKMetalView * getMirrorView() const;
    
    int createFramebuffer(int context_id, bool useColorManagement) override;
    void pushFramebuffer(int context_id, int framebuffer_id) override;
    void popFramebuffer(int context_id) override;
    void flushFramebuffer(int context_id, int framebuffer_id) override;
    void presentFramebuffer(int src_context_id, int framebuffer_id, int dst_context_id) override;
    void releaseFramebuffer(int context_id, int framebuffer_id) override;
    
protected:
    void releaseFramebuffers();
    
    NSMutableArray<MWKOpenGLContext *> *contexts;
    NSMutableArray<MWKMetalView *> *views;
#if TARGET_OS_OSX
    NSMutableArray<NSWindow *> *windows;
#else
    NSMutableArray<UIWindow *> *windows;
#endif
    
private:
    struct FramebufferStack {
        FramebufferStack(MWKMetalView *view, MWKOpenGLContext *context);
        
        int createFramebuffer();
        void pushFramebuffer(int framebuffer_id);
        void popFramebuffer();
        void bindCurrentFramebuffer() const;
        id<MTLTexture> getFramebufferTexture(int framebuffer_id) const;
        void releaseFramebuffer(int framebuffer_id);
        
    private:
        using CVPixelBufferPoolPtr = cf::ObjectPtr<CVPixelBufferPoolRef>;
        using CVPixelBufferPtr = cf::ObjectPtr<CVPixelBufferRef>;
        using CVMetalTextureCachePtr = cf::ObjectPtr<CVMetalTextureCacheRef>;
        using CVMetalTexturePtr = cf::ObjectPtr<CVMetalTextureRef>;
#if TARGET_OS_OSX
        using CVOpenGLTextureCachePtr = cf::ObjectPtr<CVOpenGLTextureCacheRef>;
        using CVOpenGLTexturePtr = cf::ObjectPtr<CVOpenGLTextureRef>;
#else
        using CVOpenGLTextureCachePtr = cf::ObjectPtr<CVOpenGLESTextureCacheRef>;
        using CVOpenGLTexturePtr = cf::ObjectPtr<CVOpenGLESTextureRef>;
#endif
        
        struct Framebuffer {
            CVPixelBufferPtr cvPixelBuffer;
            CVMetalTexturePtr cvMetalTexture;
            CVOpenGLTexturePtr cvOpenGLTexture;
            GLuint glFramebuffer = 0;
        };
        
        std::size_t width;
        std::size_t height;
        CVPixelBufferPoolPtr cvPixelBufferPool;
        CVMetalTextureCachePtr cvMetalTextureCache;
        CVOpenGLTextureCachePtr cvOpenGLTextureCache;
        std::map<int, Framebuffer> framebuffers;
        std::vector<Framebuffer> stack;
    };
    
    FramebufferStack & getFramebufferStack(int context_id) {
        return framebufferStacks.try_emplace(context_id, views[context_id], contexts[context_id]).first->second;
    }
    
    std::map<int, FramebufferStack> framebufferStacks;
    
};


END_NAMESPACE_MW


#endif /* AppleOpenGLContextManager_hpp */
