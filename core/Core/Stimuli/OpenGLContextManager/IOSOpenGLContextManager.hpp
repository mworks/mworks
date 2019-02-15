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
#include "CFObjectPtr.h"


BEGIN_NAMESPACE_MW


class IOSOpenGLContextManager : public AppleOpenGLContextManager {
    
public:
    using AppleOpenGLContextManager::AppleOpenGLContextManager;
    ~IOSOpenGLContextManager();
    
    int newFullscreenContext(int screen_number, bool render_at_full_resolution, bool opaque) override;
    int newMirrorContext(bool render_at_full_resolution) override;
    
    void releaseContexts() override;
    
    int getNumDisplays() const override;
    
    OpenGLContextLock setCurrent(int context_id) override;
    void clearCurrent() override;
    
    int createFramebufferTexture(int context_id, int width, int height, bool srgb) override;
    void bindDefaultFramebuffer(int context_id) override;
    void flush(int context_id) override;
    
    std::vector<float> getColorConversionLUTData(int context_id, int numGridPoints) override;
    
private:
    using CVPixelBufferPtr = cf::ObjectPtr<CVPixelBufferRef>;
    using CVTextureCachePtr = cf::ObjectPtr<CVOpenGLESTextureCacheRef>;
    using CVTexturePtr = cf::ObjectPtr<CVOpenGLESTextureRef>;
    
    std::map<int, CVPixelBufferPtr> cvPixelBuffers;
    std::map<int, CVTextureCachePtr> cvTextureCaches;
    std::map<int, CVTexturePtr> cvTextures;
    
};


END_NAMESPACE_MW


#endif /* IOSOpenGLContextManager_hpp */




























