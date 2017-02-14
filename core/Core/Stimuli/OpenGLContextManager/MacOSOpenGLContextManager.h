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
    ~MacOSOpenGLContextManager();
    
    int newFullscreenContext(int screen_number) override;
    int newMirrorContext() override;
    
    void releaseContexts() override;
    
    int getNumDisplays() const override;
    
    OpenGLContextLock makeCurrent(NSOpenGLContext *context) override;
    OpenGLContextLock setCurrent(int context_id) override;
    void clearCurrent() override;
    
    void flush(int context_id) override;
    
private:
    IOPMAssertionID display_sleep_block;
    
};


END_NAMESPACE_MW


#endif


























