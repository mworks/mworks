/**
 *  OpenGLContextManager.h
 *
 *  Created by David Cox on Thu Dec 05 2002.
 *  Copyright (c) 2002 MIT. All rights reserved.
 */

#ifndef OPENGL_CONTEXT_MANAGER__
#define OPENGL_CONTEXT_MANAGER__

#include <Cocoa/Cocoa.h>
#include <IOKit/pwr_mgt/IOPMLib.h>

#include "OpenGLContextLock.h"
#include "RegisteredSingleton.h"


BEGIN_NAMESPACE_MW


class OpenGLContextManager : public Component {
    
protected:
    NSMutableArray			*contexts;
    
    NSWindow				*mirror_window;
    NSOpenGLView            *mirror_view;
    
    NSWindow                *fullscreen_window;
    NSOpenGLView            *fullscreen_view;
    
    IOPMAssertionID         display_sleep_block;
    
    NSScreen                *_getScreen(const int screen_number);
    
public:
    OpenGLContextManager();
    
    // Create a "mirror" window (smaller, movable window that displays whatever
    // is on the "main" display) and return its context index
    int newMirrorContext();
    
    // Create a fullscreen context on a particular display
    int newFullscreenContext(int index);
    
    // "Let go" of captured fullscreen contexts
    void releaseDisplays();
    
    NSOpenGLContext * getContext(int context_id) const;
    NSOpenGLView * getFullscreenView() const { return fullscreen_view; }
    NSOpenGLView * getMirrorView() const { return mirror_view; }
    
    // Get information about the monitors attached to the system
    int getNMonitors();
    
    OpenGLContextLock makeCurrent(NSOpenGLContext *context);
    OpenGLContextLock setCurrent(int context_id);
    
    void flush(int context_id);
    
    REGISTERED_SINGLETON_CODE_INJECTION(OpenGLContextManager)
    
};


END_NAMESPACE_MW


#endif
