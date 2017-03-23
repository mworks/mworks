/**
 * OpenGLContextManager.cpp
 *
 * Created by David Cox on Thu Dec 05 2002.
 * Copyright (c) 2002 MIT. All rights reserved.
 */

#include "OpenGLContextManager.h"


BEGIN_NAMESPACE_MW


#if TARGET_OS_OSX


OpenGLContextLock::~OpenGLContextLock() {
    if (contextObj) {
        OpenGLContextManager::instance()->clearCurrent();
        unlock();
    }
}


OpenGLContextLock::OpenGLContextLock(CGLContextObj contextObj) :
    contextObj(contextObj)
{
    if (contextObj) {
        CGLError error = CGLLockContext(contextObj);
        if (kCGLNoError != error) {
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN,
                                  (boost::format("Unable to lock OpenGL context (error = %d)") % error).str());
        }
    }
}


OpenGLContextLock::OpenGLContextLock(OpenGLContextLock &&other) :
    contextObj(other.contextObj)
{
    other.contextObj = nullptr;
}


OpenGLContextLock& OpenGLContextLock::operator=(OpenGLContextLock &&other) {
    if (this != &other) {
        if (contextObj) {
            unlock();
        }
        contextObj = other.contextObj;
        other.contextObj = nullptr;
    }
    return (*this);
}


void OpenGLContextLock::unlock() {
    CGLError error = CGLUnlockContext(contextObj);
    if (kCGLNoError != error) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "Unable to unlock OpenGL context (error = %d)", error);
    }
}


#else  // TARGET_OS_OSX


OpenGLContextLock::~OpenGLContextLock() {
    if (lock) {
        OpenGLContextManager::instance()->clearCurrent();
    }
}


OpenGLContextLock::OpenGLContextLock(unique_lock lock) :
    lock(std::move(lock))
{ }


OpenGLContextLock::OpenGLContextLock(OpenGLContextLock &&other) :
    lock(std::move(other.lock))
{ }


OpenGLContextLock& OpenGLContextLock::operator=(OpenGLContextLock &&other) {
    if (this != &other) {
        lock = std::move(other.lock);
    }
    return (*this);
}


#endif // TARGET_OS_OSX


OpenGLContextManager::OpenGLContextManager() :
    contexts([[NSMutableArray alloc] init]),
    views([[NSMutableArray alloc] init]),
    windows([[NSMutableArray alloc] init])
#if TARGET_OS_OSX
    , display_sleep_block(kIOPMNullAssertionID)
#endif
{ }


OpenGLContextManager::~OpenGLContextManager() {
    // Calling releaseContexts here causes the application to crash at exit.  Since this class is
    // used as a singleton, it doesn't matter, anyway.
    //releaseContexts();
    
    [windows release];
    [views release];
    [contexts release];
}


auto OpenGLContextManager::getContext(int context_id) const -> PlatformOpenGLContextPtr {
    if (context_id < 0 || context_id >= contexts.count) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "OpenGL Context Manager: invalid context ID: %d", context_id);
        return nil;
    }
    return contexts[context_id];
}


auto OpenGLContextManager::getView(int context_id) const -> PlatformOpenGLViewPtr {
    if (context_id < 0 || context_id >= views.count) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "OpenGL Context Manager: invalid context ID: %d", context_id);
        return nil;
    }
    return views[context_id];
}


auto OpenGLContextManager::getFullscreenView() const -> PlatformOpenGLViewPtr {
    if (views.count > 0) {
        return views[0];
    }
    return nil;
}


auto OpenGLContextManager::getMirrorView() const -> PlatformOpenGLViewPtr {
    if (views.count > 1) {
        return views[1];
    }
    return getFullscreenView();
}


SINGLETON_INSTANCE_STATIC_DECLARATION(OpenGLContextManager)


END_NAMESPACE_MW


























