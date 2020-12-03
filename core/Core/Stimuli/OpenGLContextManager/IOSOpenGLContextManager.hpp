//
//  IOSOpenGLContextManager.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 2/14/17.
//
//

#ifndef IOSOpenGLContextManager_hpp
#define IOSOpenGLContextManager_hpp

#include "AppleOpenGLContextManager.hpp"


BEGIN_NAMESPACE_MW


class IOSOpenGLContextManager : public AppleOpenGLContextManager {
    
public:
    IOSOpenGLContextManager();
    ~IOSOpenGLContextManager();
    
    int newFullscreenContext(int screen_number, bool opaque) override;
    int newMirrorContext() override;
    
    void releaseContexts() override;
    
    int getNumDisplays() const override;
    
    OpenGLContextLock setCurrent(int context_id) override;
    void clearCurrent() override;
    
    void prepareContext(int context_id, bool useColorManagement) override;
    
private:
    id<MTLDevice> metalDevice;
    
};


END_NAMESPACE_MW


#endif /* IOSOpenGLContextManager_hpp */
