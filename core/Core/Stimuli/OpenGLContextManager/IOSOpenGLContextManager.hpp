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
    IOSOpenGLContextManager();
    ~IOSOpenGLContextManager();
    
    int newFullscreenContext(int screen_number, bool render_at_full_resolution, bool opaque) override;
    int newMirrorContext(bool render_at_full_resolution) override;
    
    void releaseContexts() override;
    
    int getNumDisplays() const override;
    
    OpenGLContextLock setCurrent(int context_id) override;
    void clearCurrent() override;
    
    void prepareContext(int context_id, bool useColorManagement) override;
    int createFramebufferTexture(int context_id, int width, int height, bool srgb) override;
    void flushFramebufferTexture(int context_id) override;
    void drawFramebufferTexture(int src_context_id, int dst_context_id) override;
    
private:
    using CVPixelBufferPtr = cf::ObjectPtr<CVPixelBufferRef>;
    using CVMetalTextureCachePtr = cf::ObjectPtr<CVMetalTextureCacheRef>;
    using CVMetalTexturePtr = cf::ObjectPtr<CVMetalTextureRef>;
    using CVOpenGLESTextureCachePtr = cf::ObjectPtr<CVOpenGLESTextureCacheRef>;
    using CVOpenGLESTexturePtr = cf::ObjectPtr<CVOpenGLESTextureRef>;
    
    void checkDisplayGamut(int context_id) const;
    
    id<MTLDevice> metalDevice;
    
    std::map<int, CVPixelBufferPtr> cvPixelBuffers;
    std::map<int, CVMetalTextureCachePtr> cvMetalTextureCaches;
    std::map<int, CVMetalTexturePtr> cvMetalTextures;
    std::map<int, CVOpenGLESTextureCachePtr> cvOpenGLESTextureCaches;
    std::map<int, CVOpenGLESTexturePtr> cvOpenGLESTextures;
    
};


END_NAMESPACE_MW


#endif /* IOSOpenGLContextManager_hpp */




























