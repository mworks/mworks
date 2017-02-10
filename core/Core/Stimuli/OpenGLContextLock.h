//
//  OpenGLContextLock.h
//  MWorksCore
//
//  Created by Christopher Stawarz on 2/14/13.
//
//

#ifndef MWorksCore_OpenGLContextLock_h
#define MWorksCore_OpenGLContextLock_h

#include <TargetConditionals.h>
#if TARGET_OS_OSX
#  include <OpenGL/CGLTypes.h>
#else
#  include <mutex>
#endif

#include "MWorksMacros.h"


BEGIN_NAMESPACE_MW


class OpenGLContextLock {
    
public:
    ~OpenGLContextLock() {
        unlock(true);
    }
    
#if TARGET_OS_OSX
    explicit OpenGLContextLock(CGLContextObj contextObj = nullptr);
#else
    using unique_lock = std::unique_lock<std::mutex>;
    explicit OpenGLContextLock(unique_lock lock = unique_lock());
#endif
    
    // No copying
    OpenGLContextLock(const OpenGLContextLock &other) = delete;
    OpenGLContextLock& operator=(const OpenGLContextLock &other) = delete;
    
    // Move constructor
    OpenGLContextLock(OpenGLContextLock &&other);
    
    // Move assignment
    OpenGLContextLock& operator=(OpenGLContextLock &&other);
    
private:
    void unlock(bool clearCurrent);
    
#if TARGET_OS_OSX
    CGLContextObj contextObj;
#else
    unique_lock lock;
#endif
    
};


END_NAMESPACE_MW


#endif  // !defined(MWorksCore_OpenGLContextLock_h)


























