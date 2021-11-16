//
//  AppleOpenGLContextManager.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 3/27/17.
//
//

#include "AppleOpenGLContextManager.hpp"

#include "OpenGLUtilities.hpp"


#if MWORKS_HAVE_OPENGL
@implementation MWKOpenGLContext {
    mw::OpenGLContextLock::unique_lock::mutex_type mutex;
}


- (mw::OpenGLContextLock)lockContext {
    return mw::OpenGLContextLock(mw::OpenGLContextLock::unique_lock(mutex));
}


@end
#endif // MWORKS_HAVE_OPENGL


BEGIN_NAMESPACE_MW


AppleOpenGLContextManager::AppleOpenGLContextManager() :
#if MWORKS_HAVE_OPENGL
    contexts(nil),
#endif
    views(nil),
    windows(nil)
{
    @autoreleasepool {
#if MWORKS_HAVE_OPENGL
        contexts = [[NSMutableArray alloc] init];
#endif
        views = [[NSMutableArray alloc] init];
        windows = [[NSMutableArray alloc] init];
    }
}


AppleOpenGLContextManager::~AppleOpenGLContextManager() {
    @autoreleasepool {
        // Set these to nil, so that ARC releases them inside the autorelease pool
        windows = nil;
        views = nil;
#if MWORKS_HAVE_OPENGL
        contexts = nil;
#endif
    }
}


#if MWORKS_HAVE_OPENGL
MWKOpenGLContext * AppleOpenGLContextManager::getContext(int context_id) const  {
    @autoreleasepool {
        if (context_id < 0 || context_id >= contexts.count) {
            merror(M_DISPLAY_MESSAGE_DOMAIN, "OpenGL Context Manager: invalid context ID: %d", context_id);
            return nil;
        }
        return contexts[context_id];
    }
}
#endif // MWORKS_HAVE_OPENGL


MTKView * AppleOpenGLContextManager::getView(int context_id) const  {
    @autoreleasepool {
        if (context_id < 0 || context_id >= views.count) {
            merror(M_DISPLAY_MESSAGE_DOMAIN, "OpenGL Context Manager: invalid context ID: %d", context_id);
            return nil;
        }
        return views[context_id];
    }
}


END_NAMESPACE_MW
