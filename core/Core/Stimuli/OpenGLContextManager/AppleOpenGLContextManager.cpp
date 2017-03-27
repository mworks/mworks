//
//  AppleOpenGLContextManager.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 3/27/17.
//
//

#include "AppleOpenGLContextManager.hpp"


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


auto AppleOpenGLContextManager::getContext(int context_id) const -> PlatformOpenGLContextPtr {
    @autoreleasepool {
        if (context_id < 0 || context_id >= contexts.count) {
            merror(M_DISPLAY_MESSAGE_DOMAIN, "OpenGL Context Manager: invalid context ID: %d", context_id);
            return nil;
        }
        return contexts[context_id];
    }
}


auto AppleOpenGLContextManager::getView(int context_id) const -> PlatformOpenGLViewPtr {
    @autoreleasepool {
        if (context_id < 0 || context_id >= views.count) {
            merror(M_DISPLAY_MESSAGE_DOMAIN, "OpenGL Context Manager: invalid context ID: %d", context_id);
            return nil;
        }
        return views[context_id];
    }
}


auto AppleOpenGLContextManager::getFullscreenView() const -> PlatformOpenGLViewPtr {
    @autoreleasepool {
        if (views.count > 0) {
            return views[0];
        }
        return nil;
    }
}


auto AppleOpenGLContextManager::getMirrorView() const -> PlatformOpenGLViewPtr {
    @autoreleasepool {
        if (views.count > 1) {
            return views[1];
        }
        return getFullscreenView();
    }
}


END_NAMESPACE_MW


























