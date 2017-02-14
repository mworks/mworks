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
    IOSOpenGLContextManager();
    ~IOSOpenGLContextManager();
    
    int newFullscreenContext(int index) override;
    int newMirrorContext() override;
    
    void releaseDisplays() override;
    
    EAGLContext * getContext(int context_id) const override;
    GLKView * getFullscreenView() const override { return fullscreenView; }
    GLKView * getMirrorView() const override { return nil; }
    
    int getNMonitors() const override { return 1; }
    
    OpenGLContextLock makeCurrent(EAGLContext *context) override;
    OpenGLContextLock setCurrent(int context_id) override;
    void clearCurrent() override;
    
    void flush(int context_id) override;
    
private:
    NSMutableArray<EAGLContext *> *contexts;
    GLKView *fullscreenView;
    
    using unique_lock = OpenGLContextLock::unique_lock;
    std::unordered_map<EAGLContext *, unique_lock::mutex_type> mutexes;
    
};


END_NAMESPACE_MW


#endif /* IOSOpenGLContextManager_hpp */




























