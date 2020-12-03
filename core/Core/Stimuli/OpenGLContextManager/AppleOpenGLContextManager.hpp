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
    
    int createFramebufferTexture(int context_id, bool useColorManagement, int &target, int &width, int &height) override;
    void flushFramebufferTexture(int context_id) override;
    void drawFramebufferTexture(int src_context_id, int dst_context_id) override;
    
protected:
    void releaseFramebufferTextures();
    
    NSMutableArray<MWKOpenGLContext *> *contexts;
    NSMutableArray<MWKMetalView *> *views;
#if TARGET_OS_OSX
    NSMutableArray<NSWindow *> *windows;
#else
    NSMutableArray<UIWindow *> *windows;
#endif
    
private:
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
    
    std::map<int, CVPixelBufferPtr> cvPixelBuffers;
    std::map<int, CVMetalTextureCachePtr> cvMetalTextureCaches;
    std::map<int, CVMetalTexturePtr> cvMetalTextures;
    std::map<int, CVOpenGLTextureCachePtr> cvOpenGLTextureCaches;
    std::map<int, CVOpenGLTexturePtr> cvOpenGLTextures;
    
};


END_NAMESPACE_MW


#endif /* AppleOpenGLContextManager_hpp */
