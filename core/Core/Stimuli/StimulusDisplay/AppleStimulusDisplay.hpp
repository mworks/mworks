//
//  AppleStimulusDisplay.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 12/14/20.
//

#ifndef AppleStimulusDisplay_hpp
#define AppleStimulusDisplay_hpp

#include "AppleOpenGLContextManager.hpp"
#include "CFObjectPtr.h"
#include "StimulusDisplay.h"


BEGIN_NAMESPACE_MW


class AppleStimulusDisplay : public StimulusDisplay {
    
public:
    explicit AppleStimulusDisplay(bool useColorManagement);
    ~AppleStimulusDisplay();
    
    MTKView * getMainView() const { return mainView; }
    MTKView * getMirrorView() const { return (mirrorView ? mirrorView : mainView); }
    
    void setRenderingMode(RenderingMode mode) override;
    
    int createFramebuffer() override;
    void pushFramebuffer(int framebuffer_id) override;
    void bindCurrentFramebuffer() override;
    void popFramebuffer() override;
    void releaseFramebuffer(int framebuffer_id) override;
    
    id<MTLDevice> getMetalDevice() const { return device; }
    id<MTLCommandQueue> getMetalCommandQueue() const { return commandQueue; }
    MTLPixelFormat getMetalFramebufferTexturePixelFormat() const { return MTLPixelFormatRGBA16Float; }
    id<MTLTexture> getMetalFramebufferTexture(int framebuffer_id) const;
    
    id<MTLTexture> getCurrentMetalFramebufferTexture() const { return currentFramebufferTexture; }
    id<MTLCommandBuffer> getCurrentMetalCommandBuffer() const { return currentCommandBuffer; }
    MTLRenderPassDescriptor * createMetalRenderPassDescriptor(MTLLoadAction loadAction = MTLLoadActionLoad,
                                                              MTLStoreAction storeAction = MTLStoreActionStore) const;
    
protected:
    void prepareContext(int context_id, bool isMainContext) override;
    void renderDisplay(bool needDraw, const std::vector<boost::shared_ptr<Stimulus>> &stimsToDraw) override;
    
private:
    using CVPixelBufferPoolPtr = cf::ObjectPtr<CVPixelBufferPoolRef>;
    using CVPixelBufferPtr = cf::ObjectPtr<CVPixelBufferRef>;
    using CVMetalTextureCachePtr = cf::ObjectPtr<CVMetalTextureCacheRef>;
    using CVMetalTexturePtr = cf::ObjectPtr<CVMetalTextureRef>;
#if TARGET_OS_OSX
    using CVOpenGLTextureCachePtr = cf::ObjectPtr<CVOpenGLTextureCacheRef>;
    using CVOpenGLTexturePtr = cf::ObjectPtr<CVOpenGLTextureRef>;
#else
    using CVOpenGLTextureCachePtr = cf::ObjectPtr<CVOpenGLESTextureCacheRef>;
    using CVOpenGLTexturePtr = cf::ObjectPtr<CVOpenGLESTextureRef>;
#endif
    
    struct Framebuffer {
        CVPixelBufferPtr cvPixelBuffer;
        CVMetalTexturePtr cvMetalTexture;
        CVOpenGLTexturePtr cvOpenGLTexture;
        GLuint glFramebuffer = 0;
    };
    
    void prepareFramebufferStack(MTKView *view, MWKOpenGLContext *context);
    void bindFramebuffer(Framebuffer &framebuffer);
    
    bool inOpenGLMode() const { return (currentRenderingMode == RenderingMode::OpenGL); }
    
    id<MTLDevice> device;
    id<MTLCommandQueue> commandQueue;
    MTKView *mainView;
    MTKView *mirrorView;
    id<MTKViewDelegate> mainViewDelegate;
    id<MTKViewDelegate> mirrorViewDelegate;
    
    std::size_t framebufferWidth;
    std::size_t framebufferHeight;
    CVPixelBufferPoolPtr cvPixelBufferPool;
    CVMetalTextureCachePtr cvMetalTextureCache;
    CVOpenGLTextureCachePtr cvOpenGLTextureCache;
    std::map<int, Framebuffer> framebuffers;
    std::vector<int> framebufferStack;
    
    int framebuffer_id;
    
    RenderingMode currentRenderingMode;
    id<MTLTexture> currentFramebufferTexture;
    id<MTLCommandBuffer> currentCommandBuffer;
    
};


inline MTLRenderPassDescriptor *
AppleStimulusDisplay::createMetalRenderPassDescriptor(MTLLoadAction loadAction, MTLStoreAction storeAction) const {
    if (!currentFramebufferTexture) {
        return nil;
    }
    MTLRenderPassDescriptor *renderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
    renderPassDescriptor.colorAttachments[0].texture = currentFramebufferTexture;
    renderPassDescriptor.colorAttachments[0].loadAction = loadAction;
    renderPassDescriptor.colorAttachments[0].storeAction = storeAction;
    return renderPassDescriptor;
}


END_NAMESPACE_MW


#endif /* AppleStimulusDisplay_hpp */
