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
    
    id<MTLDevice> getMetalDevice() const { return device; }
    id<MTLCommandQueue> getMetalCommandQueue() const { return commandQueue; }
    
    MTKView * getMainView() const { return mainView; }
    MTKView * getMirrorView() const { return (mirrorView ? mirrorView : mainView); }
    
    int createFramebuffer() { return createFramebuffer(defaultFramebufferWidth, defaultFramebufferHeight); }
    int createFramebuffer(std::size_t width, std::size_t height);
    void pushFramebuffer(int framebufferID);
    void popFramebuffer();
    void releaseFramebuffer(int framebufferID);
    
    MTLTextureType getMetalFramebufferTextureType() const {
        return (getUseAntialiasing() ? MTLTextureType2DMultisample : MTLTextureType2D);
    }
    MTLPixelFormat getMetalFramebufferTexturePixelFormat() const { return MTLPixelFormatRGBA16Float; }
    NSUInteger getMetalFramebufferTextureSampleCount() const {
        return (getUseAntialiasing() ? 4 : 1);  // All devices support a sample count of 4 (and 1)
    }
    id<MTLTexture> getMetalFramebufferTexture(int framebufferID) const;
    
    id<MTLTexture> getMetalMultisampleResolveTexture() const { return multisampleResolveTexture; }
    
    MTLPixelFormat getMetalDepthTexturePixelFormat() const { return MTLPixelFormatDepth32Float; }
    id<MTLTexture> getMetalDepthTexture() const { return depthTexture; }
    
    id<MTLCommandBuffer> getCurrentMetalCommandBuffer() const { return currentCommandBuffer; }
    MTLRenderPipelineDescriptor * createMetalRenderPipelineDescriptor(id<MTLFunction> vertexFunction,
                                                                      id<MTLFunction> fragmentFunction) const;
    MTLRenderPassDescriptor * createMetalRenderPassDescriptor(MTLLoadAction loadAction = MTLLoadActionLoad,
                                                              MTLStoreAction storeAction = MTLStoreActionStore) const;
    id<MTLRenderCommandEncoder> createMetalRenderCommandEncoder(MTLRenderPassDescriptor *renderPassDescriptor) const;
    
    const MTLViewport & getDefaultMetalViewport() const { return defaultViewport; }
    void pushMetalViewport(double originX, double originY, double width, double height);
    const MTLViewport & getCurrentMetalViewport() const {
        return (viewportStack.empty() ? defaultViewport : viewportStack.back());
    }
    void popMetalViewport();
    
    const simd::float4x4 & getDefaultMetalProjectionMatrix() const { return defaultProjectionMatrix; }
    void pushMetalProjectionMatrix(double left, double right, double bottom, double top);
    const simd::float4x4 & getCurrentMetalProjectionMatrix() const {
        return (projectionMatrixStack.empty() ? defaultProjectionMatrix : projectionMatrixStack.back());
    }
    void popMetalProjectionMatrix();
    
    void getDefaultFullscreenStimulusSize(double &width, double &height) const {
        std::tie(width, height) = defaultFullscreenStimulusSize;
    }
    void getCurrentFullscreenStimulusSize(double &width, double &height) const {
        std::tie(width, height) = (fullscreenStimulusSizeStack.empty() ?
                                   defaultFullscreenStimulusSize :
                                   fullscreenStimulusSizeStack.back());
    }
    
    double convertDisplayUnitsToPixels(double size) const { return (pixelsPerDisplayUnit * std::max(0.0, size)); }
    void getCurrentTextureSizeForDisplayArea(std::size_t &textureWidth, std::size_t &textureHeight) const {
        return getCurrentTextureSizeForDisplayArea(true, 0.0, 0.0, textureWidth, textureHeight);
    }
    void getCurrentTextureSizeForDisplayArea(double displayWidth,
                                             double displayHeight,
                                             std::size_t &textureWidth,
                                             std::size_t &textureHeight) const
    {
        return getCurrentTextureSizeForDisplayArea(false, displayWidth, displayHeight, textureWidth, textureHeight);
    }
    void getCurrentTextureSizeForDisplayArea(bool fullDisplay,
                                             double displayWidth,
                                             double displayHeight,
                                             std::size_t &textureWidth,
                                             std::size_t &textureHeight) const;
    
    void configureCapture(const std::string &format, int heightPixels, const VariablePtr &enabled) override;
    
protected:
    void prepareContext(int context_id, bool isMainContext) override;
    void renderDisplay(bool needDraw, const std::vector<boost::shared_ptr<Stimulus>> &stimsToDraw) override;
    
private:
    using CVPixelBufferPtr = cf::ObjectPtr<CVPixelBufferRef>;
    using CVMetalTexturePtr = cf::ObjectPtr<CVMetalTextureRef>;
    using CGColorSpacePtr = cf::ObjectPtr<CGColorSpaceRef>;
    
    struct Framebuffer {
        Framebuffer() : texture(nil) { }
        id<MTLTexture> texture;
    };
    
    static MTLViewport createViewport(double originX, double originY, double width, double height);
    static simd::float4x4 createProjectionMatrix(double left, double right, double bottom, double top);
    
    void captureCurrentFrame();
    
    id<MTLDevice> device;
    id<MTLCommandQueue> commandQueue;
    
    MTKView *mainView;
    MTKView *mirrorView;
    MWKStimulusDisplayViewDelegate *mainViewDelegate;
    MWKStimulusDisplayViewDelegate *mirrorViewDelegate;
    
    std::size_t defaultFramebufferWidth;
    std::size_t defaultFramebufferHeight;
    std::map<int, Framebuffer> framebuffers;
    std::vector<int> framebufferStack;
    int defaultFramebufferID;
    
    id<MTLTexture> multisampleResolveTexture;
    id<MTLTexture> depthTexture;
    
    id<MTLTexture> currentFramebufferTexture;
    id<MTLCommandBuffer> currentCommandBuffer;
    
    MTLViewport defaultViewport;
    std::vector<MTLViewport> viewportStack;
    
    simd::float4x4 defaultProjectionMatrix;
    std::vector<simd::float4x4> projectionMatrixStack;
    
    std::tuple<double, double> defaultFullscreenStimulusSize;
    std::vector<std::tuple<double, double>> fullscreenStimulusSizeStack;
    
    double pixelsPerDisplayUnit;
    
    class FrameCaptureManager;
    std::unique_ptr<FrameCaptureManager> captureManager;
    
};


inline MTLRenderPipelineDescriptor *
AppleStimulusDisplay::createMetalRenderPipelineDescriptor(id<MTLFunction> vertexFunction,
                                                          id<MTLFunction> fragmentFunction) const
{
    auto renderPipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    renderPipelineDescriptor.vertexFunction = vertexFunction;
    renderPipelineDescriptor.fragmentFunction = fragmentFunction;
    renderPipelineDescriptor.colorAttachments[0].pixelFormat = getMetalFramebufferTexturePixelFormat();
#if TARGET_OS_IPHONE
    renderPipelineDescriptor.rasterSampleCount = getMetalFramebufferTextureSampleCount();
#else
    renderPipelineDescriptor.sampleCount = getMetalFramebufferTextureSampleCount();
#endif
    return renderPipelineDescriptor;
}


inline MTLRenderPassDescriptor *
AppleStimulusDisplay::createMetalRenderPassDescriptor(MTLLoadAction loadAction, MTLStoreAction storeAction) const {
    if (!currentFramebufferTexture) {
        return nil;
    }
    auto renderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
    renderPassDescriptor.colorAttachments[0].texture = currentFramebufferTexture;
    renderPassDescriptor.colorAttachments[0].loadAction = loadAction;
    renderPassDescriptor.colorAttachments[0].storeAction = storeAction;
    return renderPassDescriptor;
}


inline id<MTLRenderCommandEncoder>
AppleStimulusDisplay::createMetalRenderCommandEncoder(MTLRenderPassDescriptor *renderPassDescriptor) const {
    auto renderCommandEncoder = [currentCommandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
    if (!viewportStack.empty()) {
        [renderCommandEncoder setViewport:viewportStack.back()];
    }
    return renderCommandEncoder;
}


inline void
AppleStimulusDisplay::getCurrentTextureSizeForDisplayArea(bool fullDisplay,
                                                          double displayWidth,
                                                          double displayHeight,
                                                          std::size_t &textureWidth,
                                                          std::size_t &textureHeight) const
{
    if (fullDisplay) {
        textureWidth = defaultFramebufferWidth;
        textureHeight = defaultFramebufferHeight;
    } else {
        // Ensure that texture contains at least one texel
        textureWidth = std::max(1.0, convertDisplayUnitsToPixels(displayWidth));
        textureHeight = std::max(1.0, convertDisplayUnitsToPixels(displayHeight));
    }
    // Limit the texture size to the size of the current viewport
    auto &currentViewport = getCurrentMetalViewport();
    textureWidth = std::min(textureWidth, std::size_t(currentViewport.width));
    textureHeight = std::min(textureHeight, std::size_t(currentViewport.height));
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
    using CVPixelBufferPoolPtr = cf::ObjectPtr<CVPixelBufferPoolRef>;
    using CVMetalTextureCachePtr = cf::ObjectPtr<CVMetalTextureCacheRef>;
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
