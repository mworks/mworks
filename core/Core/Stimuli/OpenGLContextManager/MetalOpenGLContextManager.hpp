//
//  MetalOpenGLContextManager.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 3/6/19.
//

#ifndef MetalOpenGLContextManager_hpp
#define MetalOpenGLContextManager_hpp

#include "AppleOpenGLContextManager.hpp"
#include "CFObjectPtr.h"

#include <MetalKit/MetalKit.h>


@interface MWKMetalView : MTKView

- (BOOL)prepare;

@end


BEGIN_NAMESPACE_MW


class MetalOpenGLContextManager : public AppleOpenGLContextManager {
    
public:
    MetalOpenGLContextManager();
    ~MetalOpenGLContextManager();
    
    int createFramebufferTexture(int context_id, int width, int height, bool srgb) override;
    void flushFramebufferTexture(int context_id) override;
    void drawFramebufferTexture(int src_context_id, int dst_context_id) override;
    
protected:
    void releaseFramebufferTextures();
    
    id<MTLDevice> metalDevice;
    
private:
    using CVPixelBufferPtr = cf::ObjectPtr<CVPixelBufferRef>;
    using CVMetalTextureCachePtr = cf::ObjectPtr<CVMetalTextureCacheRef>;
    using CVMetalTexturePtr = cf::ObjectPtr<CVMetalTextureRef>;
#if TARGET_OS_IPHONE
    using CVOpenGLTextureCachePtr = cf::ObjectPtr<CVOpenGLESTextureCacheRef>;
    using CVOpenGLTexturePtr = cf::ObjectPtr<CVOpenGLESTextureRef>;
#endif
    
    std::map<int, CVPixelBufferPtr> cvPixelBuffers;
    std::map<int, CVMetalTextureCachePtr> cvMetalTextureCaches;
    std::map<int, CVMetalTexturePtr> cvMetalTextures;
    std::map<int, CVOpenGLTextureCachePtr> cvOpenGLTextureCaches;
    std::map<int, CVOpenGLTexturePtr> cvOpenGLTextures;
    
};


END_NAMESPACE_MW


#endif /* MetalOpenGLContextManager_hpp */
