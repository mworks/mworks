//
//  AppleOpenGLContextManager.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 3/27/17.
//
//

#include "AppleOpenGLContextManager.hpp"

#if TARGET_OS_OSX
#  include "LegacyMacOSOpenGLContextManager.h"
#  include "MacOSOpenGLContextManager.hpp"
#elif TARGET_OS_IPHONE
#  include "IOSOpenGLContextManager.hpp"
#endif


@implementation MWKOpenGLContext {
    mw::OpenGLContextLock::unique_lock::mutex_type mutex;
}


- (mw::OpenGLContextLock)lockContext {
    return mw::OpenGLContextLock(mw::OpenGLContextLock::unique_lock(mutex));
}


@end


BEGIN_NAMESPACE_MW


AppleOpenGLContextManager::AppleOpenGLContextManager() :
    contexts(nil),
    views(nil),
    windows(nil)
{
    @autoreleasepool {
        contexts = [[NSMutableArray alloc] init];
        views = [[NSMutableArray alloc] init];
        windows = [[NSMutableArray alloc] init];
    }
}


AppleOpenGLContextManager::~AppleOpenGLContextManager() {
    @autoreleasepool {
        // Set these to nil, so that ARC releases them inside the autorelease pool
        windows = nil;
        views = nil;
        contexts = nil;
    }
}


MWKOpenGLContext * AppleOpenGLContextManager::getContext(int context_id) const  {
    @autoreleasepool {
        if (context_id < 0 || context_id >= contexts.count) {
            merror(M_DISPLAY_MESSAGE_DOMAIN, "OpenGL Context Manager: invalid context ID: %d", context_id);
            return nil;
        }
        return contexts[context_id];
    }
}


MWKOpenGLContext * AppleOpenGLContextManager::getFullscreenContext() const {
    @autoreleasepool {
        if (contexts.count > 0) {
            return contexts[0];
        }
        return nil;
    }
}


MWKOpenGLContext * AppleOpenGLContextManager::getMirrorContext() const  {
    @autoreleasepool {
        if (contexts.count > 1) {
            return contexts[1];
        }
        return getFullscreenContext();
    }
}


auto AppleOpenGLContextManager::getView(int context_id) const -> PlatformViewPtr {
    @autoreleasepool {
        if (context_id < 0 || context_id >= views.count) {
            merror(M_DISPLAY_MESSAGE_DOMAIN, "OpenGL Context Manager: invalid context ID: %d", context_id);
            return nil;
        }
        return views[context_id];
    }
}


auto AppleOpenGLContextManager::getFullscreenView() const -> PlatformViewPtr {
    @autoreleasepool {
        if (views.count > 0) {
            return views[0];
        }
        return nil;
    }
}


auto AppleOpenGLContextManager::getMirrorView() const -> PlatformViewPtr {
    @autoreleasepool {
        if (views.count > 1) {
            return views[1];
        }
        return getFullscreenView();
    }
}


boost::shared_ptr<OpenGLContextManager> OpenGLContextManager::createPlatformOpenGLContextManager() {
#if TARGET_OS_OSX
    @autoreleasepool {
        // Only use the modern, Metal-backed OpenGLContextManager on macOS 10.13 and later,
        // as the window server did not use Metal in earlier releases (so presumably OpenGL
        // is more efficient there)
        if (@available(macOS 10.13, *)) {
            // Check if Metal is supported by attempting to obtain the default device
            if (id<MTLDevice> metalDevice = MTLCreateSystemDefaultDevice()) {
                return boost::make_shared<MacOSOpenGLContextManager>();
            }
        }
        return boost::make_shared<LegacyMacOSOpenGLContextManager>();
    }
#elif TARGET_OS_IPHONE
    return boost::make_shared<IOSOpenGLContextManager>();
#else
#   error Unsupported platform
#endif
}


END_NAMESPACE_MW
