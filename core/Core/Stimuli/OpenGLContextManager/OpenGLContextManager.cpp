/**
 * OpenGLContextManager.cpp
 *
 * Created by David Cox on Thu Dec 05 2002.
 * Copyright (c) 2002 MIT. All rights reserved.
 */

#include "OpenGLContextManager.h"

#if TARGET_OS_OSX
#  include "MacOSOpenGLContextManager.h"
#endif


BEGIN_NAMESPACE_MW


#if TARGET_OS_OSX


OpenGLContextLock::OpenGLContextLock(CGLContextObj contextObj) :
    contextObj(contextObj)
{
    if (contextObj) {
        CGLError error = CGLLockContext(contextObj);
        if (kCGLNoError != error) {
            merror(M_DISPLAY_MESSAGE_DOMAIN, "Unable to lock OpenGL context (error = %d)", error);
        }
    }
}


OpenGLContextLock::OpenGLContextLock(OpenGLContextLock &&other) :
    contextObj(other.contextObj)
{
    other.contextObj = nullptr;
}


OpenGLContextLock& OpenGLContextLock::operator=(OpenGLContextLock &&other) {
    unlock(false);
    contextObj = other.contextObj;
    other.contextObj = nullptr;
    return (*this);
}


void OpenGLContextLock::unlock(bool clearCurrent) {
    if (contextObj) {
        CGLError error = CGLUnlockContext(contextObj);
        if (kCGLNoError != error) {
            merror(M_DISPLAY_MESSAGE_DOMAIN, "Unable to unlock OpenGL context (error = %d)", error);
        }
        if (clearCurrent) {
            OpenGLContextManager::instance()->clearCurrent();
        }
    }
}


#else  // TARGET_OS_OSX


OpenGLContextLock::OpenGLContextLock(unique_lock lock) :
    lock(std::move(lock))
{ }


OpenGLContextLock::OpenGLContextLock(OpenGLContextLock &&other) :
    lock(std::move(other.lock))
{
    other.lock = unique_lock();
}


OpenGLContextLock& OpenGLContextLock::operator=(OpenGLContextLock &&other) {
    unlock(false);
    lock = std::move(other.lock);
    other.lock = unique_lock();
    return (*this);
}


void OpenGLContextLock::unlock(bool clearCurrent) {
    if (lock) {
        lock.unlock();
        if (clearCurrent) {
            OpenGLContextManager::instance()->clearCurrent();
        }
    }
}


#endif // TARGET_OS_OSX


boost::shared_ptr<OpenGLContextManager> OpenGLContextManager::createPlatformOpenGLContextManager() {
#if TARGET_OS_OSX
    return boost::make_shared<MacOSOpenGLContextManager>();
#elif TARGET_OS_IPHONE
#   warning Need to implement IOSOpenGLContextManager!
    return nullptr;
#else
#   error Unsupported platform
#endif
}


SINGLETON_INSTANCE_STATIC_DECLARATION(OpenGLContextManager)


END_NAMESPACE_MW


























