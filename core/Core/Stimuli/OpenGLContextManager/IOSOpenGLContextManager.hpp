//
//  IOSOpenGLContextManager.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 2/14/17.
//
//

#ifndef IOSOpenGLContextManager_hpp
#define IOSOpenGLContextManager_hpp


#include "OpenGLContextManager.h"


BEGIN_NAMESPACE_MW


class IOSOpenGLContextManager : public OpenGLContextManager {
    
public:
    using OpenGLContextManager::OpenGLContextManager;
    ~IOSOpenGLContextManager();
    
    int newFullscreenContext(int screen_number) override;
    int newMirrorContext() override;
    
    void releaseContexts() override;
    
    int getNumDisplays() const override;
    
    OpenGLContextLock setCurrent(int context_id) override;
    void clearCurrent() override;
    
    void bindDefaultFramebuffer(int context_id) override;
    void flush(int context_id) override;
    
private:
    using unique_lock = OpenGLContextLock::unique_lock;
    std::unordered_map<EAGLContext *, unique_lock::mutex_type> mutexes;
    
};


END_NAMESPACE_MW


#endif /* IOSOpenGLContextManager_hpp */




























