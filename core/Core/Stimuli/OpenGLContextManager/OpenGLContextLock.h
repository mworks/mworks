//
//  OpenGLContextLock.h
//  MWorksCore
//
//  Created by Christopher Stawarz on 2/14/13.
//
//

#ifndef MWorksCore_OpenGLContextLock_h
#define MWorksCore_OpenGLContextLock_h

#include <mutex>

#include "MWorksMacros.h"


BEGIN_NAMESPACE_MW


class OpenGLContextLock {
    
public:
    using unique_lock = std::unique_lock<std::mutex>;
    
    ~OpenGLContextLock();
    
    explicit OpenGLContextLock(unique_lock lock = unique_lock()) :
        lock(std::move(lock))
    { }
    
    // No copying
    OpenGLContextLock(const OpenGLContextLock &other) = delete;
    OpenGLContextLock& operator=(const OpenGLContextLock &other) = delete;
    
    // Move constructor
    OpenGLContextLock(OpenGLContextLock &&other) :
        lock(std::move(other.lock))
    { }
    
    // Move assignment
    OpenGLContextLock& operator=(OpenGLContextLock &&other) {
        if (this != &other) {
            lock = std::move(other.lock);
        }
        return (*this);
    }
    
private:
    unique_lock lock;
    
};


END_NAMESPACE_MW


#endif  // !defined(MWorksCore_OpenGLContextLock_h)


























