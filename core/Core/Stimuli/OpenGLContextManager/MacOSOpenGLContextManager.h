/**
 *  MacOSOpenGLContextManager.h
 *
 *  Created by David Cox on Thu Dec 05 2002.
 *  Copyright (c) 2002 MIT. All rights reserved.
 */

#ifndef MACOS_OPENGL_CONTEXT_MANAGER__
#define MACOS_OPENGL_CONTEXT_MANAGER__

#include <IOKit/pwr_mgt/IOPMLib.h>

#include "AppleOpenGLContextManager.hpp"
#include "CFObjectPtr.h"


BEGIN_NAMESPACE_MW


class MacOSOpenGLContextManager : public AppleOpenGLContextManager {
    
public:
    MacOSOpenGLContextManager();
    ~MacOSOpenGLContextManager();
    
    int newFullscreenContext(int screen_number, bool render_at_full_resolution, bool opaque) override;
    int newMirrorContext(bool render_at_full_resolution) override;
    
    void releaseContexts() override;
    
    int getNumDisplays() const override;
    
    OpenGLContextLock setCurrent(int context_id) override;
    OpenGLContextLock setCurrent(NSOpenGLContext *context);
    void clearCurrent() override;
    
    int createFramebufferTexture(int context_id, int width, int height, bool srgb) override;
    void bindDefaultFramebuffer(int context_id) override;
    void flush(int context_id) override;
    
    std::vector<float> getColorConversionLUTData(int context_id, int numGridPoints) override;
    
private:
    using ColorSyncProfilePtr = cf::ObjectPtr<ColorSyncProfileRef>;
    using ColorSyncTransformPtr = cf::ObjectPtr<ColorSyncTransformRef>;
    
    static ColorSyncTransformPtr createColorSyncTransform(const ColorSyncProfilePtr &srcProfile,
                                                          const ColorSyncProfilePtr &dstProfile);
    static void getColorConversionLUTData(const ColorSyncTransformPtr &transform,
                                          std::vector<float> &lutData,
                                          int numGridPoints);
    
    IOPMAssertionID display_sleep_block;
    
};


END_NAMESPACE_MW


#endif


























