/**
 *  OpenGLContextManager.h
 *
 *  Created by David Cox on Thu Dec 05 2002.
 *  Copyright (c) 2002 MIT. All rights reserved.
 */

#ifndef OPENGL_CONTEXT_MANAGER__
#define OPENGL_CONTEXT_MANAGER__

#include "OpenGLContextLock.h"
#include "RegisteredSingleton.h"


BEGIN_NAMESPACE_MW


class OpenGLContextManager : public Component, boost::noncopyable {
    
public:
    static boost::shared_ptr<OpenGLContextManager> createPlatformOpenGLContextManager();
    
    // Create a fullscreen context on a particular display
    virtual int newFullscreenContext(int screen_number, bool render_at_full_resolution) = 0;
    
    // Create a "mirror" window (smaller, movable window that displays whatever
    // is on the "main" display) and return its context index
    virtual int newMirrorContext(bool render_at_full_resolution) = 0;
    
    // Release all contexts and associated resources
    virtual void releaseContexts() = 0;
    
    virtual int getNumDisplays() const = 0;
    
    virtual OpenGLContextLock setCurrent(int context_id) = 0;
    virtual void clearCurrent() = 0;
    
    virtual void bindDefaultFramebuffer(int context_id) = 0;
    virtual void flush(int context_id) = 0;
    
    REGISTERED_SINGLETON_CODE_INJECTION(OpenGLContextManager)
    
};


END_NAMESPACE_MW


#endif

























