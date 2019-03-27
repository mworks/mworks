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
#  define METAL_BACKING_OPT_IN_REQUIRED  1
#  if METAL_BACKING_OPT_IN_REQUIRED
#    include "ComponentRegistry.h"
#    define USE_METAL_BACKING_KEY  "use_metal_backing"
#  endif
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
        [windows release];
        [views release];
        [contexts release];
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
    @autoreleasepool {
#if TARGET_OS_OSX
#  if METAL_BACKING_OPT_IN_REQUIRED
        if (auto mainScreenInfo = ComponentRegistry::getSharedRegistry()->getVariable(MAIN_SCREEN_INFO_TAGNAME)) {
            auto value = mainScreenInfo->getValue();
            if (value.hasKey(USE_METAL_BACKING_KEY) && value.getElement(USE_METAL_BACKING_KEY).getBool()) {
#  endif
                // Only use the modern, Metal-backed OpenGLContextManager on macOS 10.13 and later,
                // as the window server did not use Metal in earlier releases (so presumably OpenGL
                // is more efficient there)
                if (@available(macOS 10.13, *)) {
                    // Check if Metal is supported by attempting to obtain the default device
                    if (id<MTLDevice> metalDevice = MTLCreateSystemDefaultDevice()) {
                        [metalDevice release];
#  if METAL_BACKING_OPT_IN_REQUIRED
                        mwarning(M_DISPLAY_MESSAGE_DOMAIN,
                                 "Metal backing of stimulus display is experimental and not recommended "
                                 "for production use");
#  endif
                        return boost::make_shared<MacOSOpenGLContextManager>();
#  if METAL_BACKING_OPT_IN_REQUIRED
                    } else {
                        merror(M_DISPLAY_MESSAGE_DOMAIN,
                               "Metal is not supported on this system. "
                               "Using OpenGL backing for stimulus display.");
#  endif
                    }
#  if METAL_BACKING_OPT_IN_REQUIRED
                } else {
                    merror(M_DISPLAY_MESSAGE_DOMAIN,
                           "Metal backing of stimulus display requires macOS 10.13 or later. "
                           "Using OpenGL backing instead.");
#  endif
                }
#  if METAL_BACKING_OPT_IN_REQUIRED
            }
        }
#  endif
        return boost::make_shared<LegacyMacOSOpenGLContextManager>();
#elif TARGET_OS_IPHONE
        return boost::make_shared<IOSOpenGLContextManager>();
#else
#  error Unsupported platform
#endif
    }
}


END_NAMESPACE_MW
