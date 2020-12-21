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


@class MWKStimulusDisplayViewDelegate;  // Forward declaration


BEGIN_NAMESPACE_MW


class AppleStimulusDisplay : public StimulusDisplay {
    
public:
    explicit AppleStimulusDisplay(bool useColorManagement);
    ~AppleStimulusDisplay();
    
    MTKView * getMainView() const { return mainView; }
    MTKView * getMirrorView() const { return (mirrorView ? mirrorView : mainView); }
    
    int createFramebuffer() override;
    void pushFramebuffer(int framebuffer_id) override;
    void bindCurrentFramebuffer() override;
    void popFramebuffer() override;
    void releaseFramebuffer(int framebuffer_id) override;
    
    id<MTLCommandQueue> getMetalCommandQueue() const { return commandQueue; }
    id<MTLTexture> getCurrentMetalFramebufferTexture() const;
    
protected:
    void prepareContext(int context_id) override;
    void prepareFramebufferStack(MTKView *view, MWKOpenGLContext *context);
    void presentFramebuffer(int framebuffer_id, int dst_context_id) override;
    
    const boost::shared_ptr<AppleOpenGLContextManager> contextManager;
    
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
    
    id<MTLCommandQueue> commandQueue;
    MTKView *mainView;
    MTKView *mirrorView;
    MWKStimulusDisplayViewDelegate *mainViewDelegate;
    MWKStimulusDisplayViewDelegate *mirrorViewDelegate;
    
    std::size_t framebufferWidth;
    std::size_t framebufferHeight;
    CVPixelBufferPoolPtr cvPixelBufferPool;
    CVMetalTextureCachePtr cvMetalTextureCache;
    CVOpenGLTextureCachePtr cvOpenGLTextureCache;
    std::map<int, Framebuffer> framebuffers;
    std::vector<Framebuffer> framebufferStack;
    
};


END_NAMESPACE_MW


#endif /* AppleStimulusDisplay_hpp */
