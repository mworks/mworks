/**
 *  MacOSOpenGLContextManager.h
 *
 *  Created by David Cox on Thu Dec 05 2002.
 *  Copyright (c) 2002 MIT. All rights reserved.
 */

#ifndef MACOS_OPENGL_CONTEXT_MANAGER__
#define MACOS_OPENGL_CONTEXT_MANAGER__

#include <IOKit/pwr_mgt/IOPMLib.h>

#include "OpenGLContextManager.h"


BEGIN_NAMESPACE_MW


class MacOSOpenGLContextManager : public OpenGLContextManager {
    
public:
    MacOSOpenGLContextManager();
    
    int newMirrorContext() override;
    int newFullscreenContext(int index) override;
    
    void releaseDisplays() override;
    
    NSOpenGLContext * getContext(int context_id) const override;
    NSOpenGLView * getFullscreenView() const override { return fullscreen_view; }
    NSOpenGLView * getMirrorView() const override { return mirror_view; }
    
    int getNMonitors() override;
    
    OpenGLContextLock makeCurrent(NSOpenGLContext *context) override;
    OpenGLContextLock setCurrent(int context_id) override;
    
    void flush(int context_id);
    
private:
    NSScreen *_getScreen(const int screen_number);
    
    NSMutableArray *contexts;
    
    NSWindow *mirror_window;
    NSOpenGLView *mirror_view;
    
    NSWindow *fullscreen_window;
    NSOpenGLView *fullscreen_view;
    
    IOPMAssertionID display_sleep_block;
    
};


END_NAMESPACE_MW


#endif


























