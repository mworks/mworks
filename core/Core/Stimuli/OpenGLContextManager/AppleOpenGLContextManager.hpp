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

@end


BEGIN_NAMESPACE_MW


class AppleOpenGLContextManager : public OpenGLContextManager {
    
public:
#if TARGET_OS_OSX
    using PlatformWindowPtr = NSWindow *;
#elif TARGET_OS_IPHONE
    using PlatformWindowPtr = UIWindow *;
#else
#   error Unsupported platform
#endif
    
    AppleOpenGLContextManager();
    ~AppleOpenGLContextManager();
    
    MWKOpenGLContext * getContext(int context_id) const;
    MWKOpenGLContext * getFullscreenContext() const;
    MWKOpenGLContext * getMirrorContext() const;
    
    MWKMetalView * getView(int context_id) const;
    MWKMetalView * getFullscreenView() const;
    MWKMetalView * getMirrorView() const;
    
protected:
    NSMutableArray<MWKOpenGLContext *> *contexts;
    NSMutableArray<MWKMetalView *> *views;
    NSMutableArray<PlatformWindowPtr> *windows;
    
};


END_NAMESPACE_MW


#endif /* AppleOpenGLContextManager_hpp */
