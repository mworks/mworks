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
    using PlatformOpenGLContextPtr = NSOpenGLContext *;
    using PlatformOpenGLViewPtr = NSOpenGLView *;
#elif TARGET_OS_IPHONE
    using PlatformOpenGLContextPtr = EAGLContext *;
    using PlatformOpenGLViewPtr = UIView *;
#else
#   error Unsupported platform
#endif
    
    AppleOpenGLContextManager();
    ~AppleOpenGLContextManager();
    
    PlatformOpenGLContextPtr getContext(int context_id) const;
    PlatformOpenGLViewPtr getView(int context_id) const;
    PlatformOpenGLViewPtr getFullscreenView() const;
    PlatformOpenGLViewPtr getMirrorView() const;
    
protected:
    NSMutableArray *contexts;
    NSMutableArray *views;
    NSMutableArray *windows;
    
};


END_NAMESPACE_MW


#endif /* AppleOpenGLContextManager_hpp */


























