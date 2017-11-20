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
    using AppleOpenGLContextManager::AppleOpenGLContextManager;
    ~IOSOpenGLContextManager();
    
    int newFullscreenContext(int screen_number, bool render_at_full_resolution) override;
    int newMirrorContext(bool render_at_full_resolution) override;
    
    void releaseContexts() override;
    
    int getNumDisplays() const override;
    
    OpenGLContextLock setCurrent(int context_id) override;
    void clearCurrent() override;
    
    void bindDefaultFramebuffer(int context_id) override;
    void flush(int context_id) override;
    
};


END_NAMESPACE_MW


#endif /* IOSOpenGLContextManager_hpp */




























