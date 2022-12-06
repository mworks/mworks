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
    
    void releaseContexts() override;
    
    int getNumDisplays() const override;
    
    OpenGLContextLock setCurrent(int context_id) override;
    void clearCurrent() override;
    
private:
    int newFullscreenContext(int screen_number, bool opaque) override;
    int newMirrorContext(double width, double height, int main_context_id) override;
    
    id<MTLDevice> metalDevice;
    
};


END_NAMESPACE_MW


#endif /* IOSOpenGLContextManager_hpp */
