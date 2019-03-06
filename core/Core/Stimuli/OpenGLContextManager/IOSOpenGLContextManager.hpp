//
//  IOSOpenGLContextManager.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 2/14/17.
//
//

#ifndef IOSOpenGLContextManager_hpp
#define IOSOpenGLContextManager_hpp

#include "CFObjectPtr.h"
#include "MetalOpenGLContextManager.hpp"


BEGIN_NAMESPACE_MW


class IOSOpenGLContextManager : public MetalOpenGLContextManager {
    
public:
    IOSOpenGLContextManager();
    ~IOSOpenGLContextManager();
    
    int newFullscreenContext(int screen_number, bool render_at_full_resolution, bool opaque) override;
    int newMirrorContext(bool render_at_full_resolution) override;
    
    void releaseContexts() override;
    
    int getNumDisplays() const override;
    
    OpenGLContextLock setCurrent(int context_id) override;
    void clearCurrent() override;
    
    void prepareContext(int context_id, bool useColorManagement) override;
    
private:
    void checkDisplayGamut(int context_id) const;
    
};


END_NAMESPACE_MW


#endif /* IOSOpenGLContextManager_hpp */
