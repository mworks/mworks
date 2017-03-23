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

#if TARGET_OS_OSX
#  include <Cocoa/Cocoa.h>
#  include <IOKit/pwr_mgt/IOPMLib.h>
#elif TARGET_OS_IPHONE
#  include <UIKit/UIKit.h>
#  include <OpenGLES/EAGL.h>
#endif


BEGIN_NAMESPACE_MW


class OpenGLContextManager : public Component {
    
public:
#if TARGET_OS_OSX
    using PlatformOpenGLContextPtr = NSOpenGLContext *;
    using PlatformOpenGLViewPtr = NSOpenGLView *;
#elif TARGET_OS_IPHONE
    using PlatformOpenGLContextPtr = EAGLContext *;
    using PlatformOpenGLViewPtr = UIView *;
#else
#   error Unsupported platform
#endif
    
    OpenGLContextManager();
    ~OpenGLContextManager();
    
    // Create a fullscreen context on a particular display
    int newFullscreenContext(int screen_number);
    
    // Create a "mirror" window (smaller, movable window that displays whatever
    // is on the "main" display) and return its context index
    int newMirrorContext();
    
    // Release all contexts and associated resources
    void releaseContexts();
    
    int getNumDisplays() const;
    
    OpenGLContextLock setCurrent(int context_id);
    void clearCurrent();
    
    void bindDefaultFramebuffer(int context_id);
    void flush(int context_id);
    
    PlatformOpenGLContextPtr getContext(int context_id) const;
    PlatformOpenGLViewPtr getView(int context_id) const;
    PlatformOpenGLViewPtr getFullscreenView() const;
    PlatformOpenGLViewPtr getMirrorView() const;
    
    REGISTERED_SINGLETON_CODE_INJECTION(OpenGLContextManager)
    
private:
    NSMutableArray *contexts;
    NSMutableArray *views;
    NSMutableArray *windows;
    
#if TARGET_OS_OSX
    IOPMAssertionID display_sleep_block;
#endif
    
};


END_NAMESPACE_MW


#endif

























