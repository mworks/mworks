//
//  AppleOpenGLContextManager.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 3/27/17.
//
//

#include "AppleOpenGLContextManager.hpp"


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


MWKMetalView * AppleOpenGLContextManager::getView(int context_id) const  {
    @autoreleasepool {
        if (context_id < 0 || context_id >= views.count) {
            merror(M_DISPLAY_MESSAGE_DOMAIN, "OpenGL Context Manager: invalid context ID: %d", context_id);
            return nil;
        }
        return views[context_id];
    }
}


MWKMetalView * AppleOpenGLContextManager::getFullscreenView() const {
    @autoreleasepool {
        if (views.count > 0) {
            return views[0];
        }
        return nil;
    }
}


MWKMetalView * AppleOpenGLContextManager::getMirrorView() const {
    @autoreleasepool {
        if (views.count > 1) {
            return views[1];
        }
        return getFullscreenView();
    }
}


END_NAMESPACE_MW
