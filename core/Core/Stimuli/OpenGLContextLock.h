//
//  OpenGLContextLock.h
//  MWorksCore
//
//  Created by Christopher Stawarz on 2/14/13.
//
//

#ifndef MWorksCore_OpenGLContextLock_h
#define MWorksCore_OpenGLContextLock_h

#include <OpenGL/CGLTypes.h>

#include <boost/move/move.hpp>

#include "MWorksMacros.h"


BEGIN_NAMESPACE_MW


class OpenGLContextLock {
    
    BOOST_MOVABLE_BUT_NOT_COPYABLE(OpenGLContextLock)
    
public:
    ~OpenGLContextLock() {
        unlock(true);
    }
    
    explicit OpenGLContextLock(CGLContextObj contextObj = NULL);
    
    // Move constructor
    OpenGLContextLock(BOOST_RV_REF(OpenGLContextLock) other) :
        contextObj(other.contextObj)
    {
        other.contextObj = NULL;
    }
    
    // Move assignment
    OpenGLContextLock& operator=(BOOST_RV_REF(OpenGLContextLock) other) {
        unlock(false);
        contextObj = other.contextObj;
        other.contextObj = NULL;
        return (*this);
    }
    
private:
    void unlock(bool clearCurrent);
    
    CGLContextObj contextObj;
    
};


END_NAMESPACE_MW


#endif  // !defined(MWorksCore_OpenGLContextLock_h)


























