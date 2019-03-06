//
//  MacOSOpenGLContextManager.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 3/6/19.
//

#ifndef MacOSOpenGLContextManager_hpp
#define MacOSOpenGLContextManager_hpp

#include <IOKit/pwr_mgt/IOPMLib.h>

#include "MetalOpenGLContextManager.hpp"


BEGIN_NAMESPACE_MW


class MacOSOpenGLContextManager : public MetalOpenGLContextManager {
    
public:
    MacOSOpenGLContextManager();
    ~MacOSOpenGLContextManager();
    
    int newFullscreenContext(int screen_number, bool render_at_full_resolution, bool opaque) override;
    int newMirrorContext(bool render_at_full_resolution) override;
    
    void releaseContexts() override;
    
    int getNumDisplays() const override;
    
    OpenGLContextLock setCurrent(int context_id) override;
    void clearCurrent() override;
    
    void prepareContext(int context_id, bool useColorManagement) override;
    
private:
    IOPMAssertionID display_sleep_block;
    
};


END_NAMESPACE_MW


#endif /* MacOSOpenGLContextManager_hpp */
