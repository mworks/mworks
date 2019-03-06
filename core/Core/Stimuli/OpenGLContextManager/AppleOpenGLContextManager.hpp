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
#  include <Cocoa/Cocoa.h>
#elif TARGET_OS_IPHONE
#  define GLES_SILENCE_DEPRECATION
#  include <UIKit/UIKit.h>
#  include <OpenGLES/EAGL.h>
#endif

#include "OpenGLContextManager.h"


BEGIN_NAMESPACE_MW


class AppleOpenGLContextManager : public OpenGLContextManager {
    
public:
#if TARGET_OS_OSX
    using PlatformContextPtr = NSOpenGLContext *;
    using PlatformViewPtr = NSView *;
    using PlatformWindowPtr = NSWindow *;
#elif TARGET_OS_IPHONE
    using PlatformContextPtr = EAGLContext *;
    using PlatformViewPtr = UIView *;
    using PlatformWindowPtr = UIWindow *;
#else
#   error Unsupported platform
#endif
    
    AppleOpenGLContextManager();
    ~AppleOpenGLContextManager();
    
    PlatformContextPtr getContext(int context_id) const;
    PlatformContextPtr getFullscreenContext() const;
    PlatformContextPtr getMirrorContext() const;
    
    PlatformViewPtr getView(int context_id) const;
    PlatformViewPtr getFullscreenView() const;
    PlatformViewPtr getMirrorView() const;
    
protected:
    NSMutableArray<PlatformContextPtr> *contexts;
    NSMutableArray<PlatformViewPtr> *views;
    NSMutableArray<PlatformWindowPtr> *windows;
    
};


END_NAMESPACE_MW


#endif /* AppleOpenGLContextManager_hpp */


























