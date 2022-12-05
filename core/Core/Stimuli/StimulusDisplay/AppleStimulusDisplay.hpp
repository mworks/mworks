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
#include "GenericVariable.h"
#include "StimulusDisplay.h"

#ifdef PRODUCT_BUNDLE_IDENTIFIER
#  define MWORKS_GET_CURRENT_BUNDLE()  [NSBundle bundleWithIdentifier:@"" PRODUCT_BUNDLE_IDENTIFIER]
#endif


@class MWKStimulusDisplayViewDelegate;


BEGIN_NAMESPACE_MW


class AppleStimulusDisplay : public StimulusDisplay {
    
public:
    explicit AppleStimulusDisplay(const Configuration &config);
    ~AppleStimulusDisplay();
    
    simd::float4x4 getMetalProjectionMatrix() const { return metalProjectionMatrix; }
    
    MTKView * getMainView() const { return mainView; }
    MTKView * getMirrorView() const { return (mirrorView ? mirrorView : mainView); }
    
    void configureCapture(const std::string &format, int heightPixels, const VariablePtr &enabled) override;
    
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
#if MWORKS_HAVE_OPENGL
    using CVOpenGLTextureCachePtr = cf::ObjectPtr<CVOpenGLTextureCacheRef>;
    using CVOpenGLTexturePtr = cf::ObjectPtr<CVOpenGLTextureRef>;
#endif
    using CGColorSpacePtr = cf::ObjectPtr<CGColorSpaceRef>;
    
    struct Framebuffer {
        CVPixelBufferPtr cvPixelBuffer;
        CVMetalTexturePtr cvMetalTexture;
#if MWORKS_HAVE_OPENGL
        CVOpenGLTexturePtr cvOpenGLTexture;
        GLuint glFramebuffer = 0;
#endif
    };
    
#if MWORKS_HAVE_OPENGL
    void prepareFramebufferStack(MTKView *view, MWKOpenGLContext *context);
#else
    void prepareFramebufferStack(MTKView *view);
#endif
    void bindFramebuffer(Framebuffer &framebuffer);
    
#if MWORKS_HAVE_OPENGL
    bool inOpenGLMode() const { return (currentRenderingMode == RenderingMode::OpenGL); }
#endif
    
    void captureCurrentFrame();
    
    simd::float4x4 metalProjectionMatrix;
    
    id<MTLDevice> device;
    id<MTLCommandQueue> commandQueue;
    MTKView *mainView;
    MTKView *mirrorView;
    MWKStimulusDisplayViewDelegate *mainViewDelegate;
    MWKStimulusDisplayViewDelegate *mirrorViewDelegate;
    
    std::size_t framebufferWidth;
    std::size_t framebufferHeight;
    CVPixelBufferPoolPtr cvPixelBufferPool;
    CVMetalTextureCachePtr cvMetalTextureCache;
#if MWORKS_HAVE_OPENGL
    CVOpenGLTextureCachePtr cvOpenGLTextureCache;
#endif
    std::map<int, Framebuffer> framebuffers;
    std::vector<int> framebufferStack;
    
    int framebuffer_id;
    
    RenderingMode currentRenderingMode;
    id<MTLTexture> currentFramebufferTexture;
    id<MTLCommandBuffer> currentCommandBuffer;
    
    class FrameCaptureManager;
    std::unique_ptr<FrameCaptureManager> captureManager;
    
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


class AppleStimulusDisplay::FrameCaptureManager : boost::noncopyable {
    
public:
    static constexpr auto pixelBufferPixelFormat = kCVPixelFormatType_32BGRA;
    static constexpr auto metalTexturePixelFormat = MTLPixelFormatBGRA8Unorm;
    
    FrameCaptureManager(std::size_t bufferWidth,
                        std::size_t bufferHeight,
                        const cf::StringPtr &imageFileType,
                        const CGColorSpacePtr &imageFileColorSpace,
                        const VariablePtr &enabled,
                        id<MTLDevice> metalDevice);
    
    bool isCaptureEnabled() const { return enabled->getValue().getBool(); }
    bool createCaptureBuffer(CVPixelBufferPtr &pixelBuffer, CVMetalTexturePtr &metalTexture);
    bool convertCaptureBufferToImageFile(const CVPixelBufferPtr &pixelBuffer, std::string &imageFile) const;
    
private:
    using CGImagePtr = cf::ObjectPtr<CGImageRef>;
    
    const std::size_t bufferWidth;
    const std::size_t bufferHeight;
    const cf::StringPtr imageFileType;
    const CGColorSpacePtr imageFileColorSpace;
    const VariablePtr enabled;
    
    CVPixelBufferPoolPtr pixelBufferPool;
    cf::DictionaryPtr pixelBufferAuxAttributes;
    CVMetalTextureCachePtr metalTextureCache;
    
};


END_NAMESPACE_MW


#endif /* AppleStimulusDisplay_hpp */
