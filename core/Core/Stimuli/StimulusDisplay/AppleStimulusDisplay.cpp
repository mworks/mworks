//
//  AppleStimulusDisplay.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 12/14/20.
//

#include "AppleStimulusDisplay.hpp"

#include <CoreServices/CoreServices.h>
#include <VideoToolbox/VideoToolbox.h>

#include "AAPLMathUtilities.h"
#include "OpenGLUtilities.hpp"
#include "StandardVariables.h"
#include "Stimulus.h"

using namespace mw::aapl_math_utilities;


@interface MWKStimulusDisplayViewDelegate : NSObject <MTKViewDelegate>

@property(nonatomic, readonly) id<MTLCommandQueue> commandQueue;
@property(nonatomic, strong) id<MTLTexture> texture;
@property(nonatomic, strong) id<MTLCommandBuffer> currentCommandBuffer;

+ (instancetype)delegateWithMTKView:(MTKView *)view
                 useColorManagement:(BOOL)useColorManagement
                              error:(__autoreleasing NSError **)error;

- (instancetype)initWithCommandQueue:(id<MTLCommandQueue>)commandQueue
                 renderPipelineState:(id<MTLRenderPipelineState>)renderPipelineState;

- (void)renderWithRenderPassDescriptor:(MTLRenderPassDescriptor *)renderPassDescriptor;

@end


@implementation MWKStimulusDisplayViewDelegate {
    id<MTLRenderPipelineState> _renderPipelineState;
}


+ (instancetype)delegateWithMTKView:(MTKView *)view
                 useColorManagement:(BOOL)useColorManagement
                              error:(__autoreleasing NSError **)error
{
    id<MTLLibrary> library = [view.device newDefaultLibraryWithBundle:MWORKS_GET_CURRENT_BUNDLE() error:error];
    if (!library) {
        return nil;
    }
    
    id<MTLFunction> vertexFunction = [library newFunctionWithName:@"MWKStimulusDisplayViewDelegate_vertexShader"];
    
    MTLFunctionConstantValues *functionConstantValues = [[MTLFunctionConstantValues alloc] init];
    const bool convertToSRGB = useColorManagement;
    [functionConstantValues setConstantValue:&convertToSRGB type:MTLDataTypeBool atIndex:0];
#if TARGET_OS_OSX
    __block bool setAlphaToOne = true;
    dispatch_sync(dispatch_get_main_queue(), ^{
        setAlphaToOne = view.window.opaque;
    });
#else
    const bool setAlphaToOne = true;  // iOS windows are always opaque
#endif
    [functionConstantValues setConstantValue:&setAlphaToOne type:MTLDataTypeBool atIndex:1];
    
    id<MTLFunction> fragmentFunction = [library newFunctionWithName:@"MWKStimulusDisplayViewDelegate_fragmentShader"
                                                     constantValues:functionConstantValues
                                                              error:error];
    if (!fragmentFunction) {
        return nil;
    }
    
    MTLRenderPipelineDescriptor *renderPipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    renderPipelineDescriptor.vertexFunction = vertexFunction;
    renderPipelineDescriptor.fragmentFunction = fragmentFunction;
    renderPipelineDescriptor.colorAttachments[0].pixelFormat = view.colorPixelFormat;
    
    id<MTLRenderPipelineState> renderPipelineState = [view.device newRenderPipelineStateWithDescriptor:renderPipelineDescriptor
                                                                                                 error:error];
    if (!renderPipelineState) {
        return nil;
    }
    
    return [[self alloc] initWithCommandQueue:[view.device newCommandQueue] renderPipelineState:renderPipelineState];
}


- (instancetype)initWithCommandQueue:(id<MTLCommandQueue>)commandQueue
                 renderPipelineState:(id<MTLRenderPipelineState>)renderPipelineState
{
    self = [super init];
    if (self) {
        _commandQueue = commandQueue;
        _renderPipelineState = renderPipelineState;
    }
    return self;
}


- (void)mtkView:(MTKView *)view drawableSizeWillChange:(CGSize)size {
    // Stimulus display views aren't resizeable, so this should never be called
}


- (void)drawInMTKView:(MTKView *)view {
    MTLRenderPassDescriptor *renderPassDescriptor = view.currentRenderPassDescriptor;
    if (renderPassDescriptor) {
        renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionDontCare;
        [self renderWithRenderPassDescriptor:renderPassDescriptor];
        [self.currentCommandBuffer presentDrawable:view.currentDrawable];
    }
    if (self.currentCommandBuffer) {
        [self.currentCommandBuffer commit];
        self.currentCommandBuffer = nil;
    }
}


- (void)renderWithRenderPassDescriptor:(MTLRenderPassDescriptor *)renderPassDescriptor {
    if (!self.currentCommandBuffer) {
        self.currentCommandBuffer = [self.commandQueue commandBuffer];
        [self.currentCommandBuffer enqueue];
    }
    
    id<MTLRenderCommandEncoder> renderCommandEncoder = [self.currentCommandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
    
    [renderCommandEncoder setRenderPipelineState:_renderPipelineState];
    [renderCommandEncoder setFragmentTexture:self.texture atIndex:0];
    [renderCommandEncoder drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];
    
    [renderCommandEncoder endEncoding];
}


@end


BEGIN_NAMESPACE_MW


AppleStimulusDisplay::AppleStimulusDisplay(bool useColorManagement) :
    StimulusDisplay(useColorManagement),
    device(nil),
    commandQueue(nil),
    mainView(nil),
    mirrorView(nil),
    mainViewDelegate(nil),
    mirrorViewDelegate(nil),
    framebufferWidth(0),
    framebufferHeight(0),
    framebuffer_id(-1),
    currentRenderingMode(RenderingMode::None),
    currentFramebufferTexture(nil),
    currentCommandBuffer(nil)
{
    //
    // Metal uses different normalized device coordinates than OpenGL, so Metal-based stimuli need to use a
    // different projection matrix than OpenGL-based ones, as explained at
    // https://metashapes.com/blog/opengl-metal-projection-matrix-problem/ .
    //
    // Also, because the y coordinate of Metal textures is flipped with respect to OpenGL textures ((0,0) is
    // at the upper-left corner, instead of lower left), we swap top and bottom when creating the projection
    // matrix.  This results in Metal-based stimuli rendering themselves in the same orientation as OpenGL-based
    // ones.  MWKStimulusDisplayViewDelegate flips y again when drawing the framebuffer texture, so everything
    // is oriented correctly on the display.
    //
    double left, right, bottom, top;
    getDisplayBounds(left, right, bottom, top);
    metalProjectionMatrix = matrix_ortho_right_hand(left, right, top,/*<--swapped-->*/bottom, -1.0, 1.0);
}


AppleStimulusDisplay::~AppleStimulusDisplay() {
    @autoreleasepool {
        currentCommandBuffer = nil;
        currentFramebufferTexture = nil;
        mirrorViewDelegate = nil;
        mainViewDelegate = nil;
        mirrorView = nil;
        mainView = nil;
        commandQueue = nil;
        device = nil;
    }
}


void AppleStimulusDisplay::setRenderingMode(RenderingMode mode) {
    if (RenderingMode::None == currentRenderingMode) {
        // We're not rendering, so do nothing
        return;
    }
    
    switch (mode) {
        case RenderingMode::None:
            // No specific mode requested, so stay in the current mode
            break;
            
        case RenderingMode::Metal:
            if (inOpenGLMode()) {
                glFlush();
                currentCommandBuffer = [commandQueue commandBuffer];
                [currentCommandBuffer enqueue];
                currentRenderingMode = RenderingMode::Metal;
            }
            break;
            
        case RenderingMode::OpenGL:
            if (!inOpenGLMode()) {
                [currentCommandBuffer commit];
                [currentCommandBuffer waitUntilScheduled];
                currentCommandBuffer = nil;
                currentRenderingMode = RenderingMode::OpenGL;
            }
            break;
    }
}


void AppleStimulusDisplay::prepareContext(int context_id, bool isMainContext) {
    @autoreleasepool {
        auto contextManager = boost::dynamic_pointer_cast<AppleOpenGLContextManager>(getContextManager());
        MTKView *view = contextManager->getView(context_id);
        view.paused = YES;
        
        MWKStimulusDisplayViewDelegate *delegate = nil;
        NSError *error = nil;
        if (!(delegate = [MWKStimulusDisplayViewDelegate delegateWithMTKView:view
                                                          useColorManagement:getUseColorManagement()
                                                                       error:&error]))
        {
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN,
                                  "Cannot create stimulus display view delegate",
                                  error.localizedDescription.UTF8String);
        }
        
        if (!isMainContext) {
            view.enableSetNeedsDisplay = YES;
            mirrorView = view;
            mirrorViewDelegate = delegate;
        } else {
            view.enableSetNeedsDisplay = NO;
            device = view.device;
            commandQueue = delegate.commandQueue;
            mainView = view;
            mainViewDelegate = delegate;
            auto ctxLock = contextManager->setCurrent(context_id);
            prepareFramebufferStack(view, contextManager->getContext(context_id));
            framebuffer_id = createFramebuffer();
        }
        
        delegate.texture = getMetalFramebufferTexture(framebuffer_id);
        view.delegate = delegate;
    }
}


void AppleStimulusDisplay::renderDisplay(bool needDraw, const std::vector<boost::shared_ptr<Stimulus>> &stimsToDraw) {
    @autoreleasepool {
        if (needDraw) {
            auto ctxLock = setCurrentOpenGLContext();
#if !MWORKS_OPENGL_ES
            // This has no effect on non-sRGB framebuffers, so we can enable it unconditionally
            gl::Enabled<GL_FRAMEBUFFER_SRGB> framebufferSRGBEnabled;
#endif
            glDisable(GL_BLEND);
            glDisable(GL_DITHER);
            
            pushFramebuffer(framebuffer_id);
            
            currentRenderingMode = RenderingMode::Metal;
            currentCommandBuffer = [commandQueue commandBuffer];
            [currentCommandBuffer enqueue];
            
            {
                MTLClearColor clearColor;
                getBackgroundColor(clearColor.red, clearColor.green, clearColor.blue, clearColor.alpha);
                auto renderPassDescriptor = createMetalRenderPassDescriptor(MTLLoadActionClear);
                renderPassDescriptor.colorAttachments[0].clearColor = clearColor;
                auto renderCommandEncoder = [currentCommandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
                [renderCommandEncoder endEncoding];
            }
            
            auto sharedThis = shared_from_this();
            for (auto &stim : stimsToDraw) {
                setRenderingMode(stim->getRenderingMode());
                stim->draw(sharedThis);
            }
            
            if (inOpenGLMode()) {
                glFlush();
                currentCommandBuffer = [commandQueue commandBuffer];
                [currentCommandBuffer enqueue];
            }
            
            // Pass the current command buffer on to the main view delegate, which will commit and
            // release the buffer after drawing to the display
            mainViewDelegate.currentCommandBuffer = currentCommandBuffer;
            
            if (mirrorView) {
                // Wait until all main view rendering commands have been scheduled on the GPU
                // before triggering a mirror window update.  This avoids the situation where,
                // when display updates are stopped, the mirror window remains frozen on the
                // last frame *before* the display was cleared.
                [currentCommandBuffer addScheduledHandler:^(id<MTLCommandBuffer> commandBuffer) {
                    dispatch_async(dispatch_get_main_queue(), ^{
#if TARGET_OS_OSX
                        mirrorView.needsDisplay = YES;
#else
                        [mirrorView setNeedsDisplay];
#endif
                    });
                }];
            }
            
            if (captureManager && captureManager->isCaptureEnabled()) {
                captureCurrentFrame();
            }
            
            currentCommandBuffer = nil;
            currentRenderingMode = RenderingMode::None;
            
            popFramebuffer();
        }
        
        [mainView draw];
    }
}


void AppleStimulusDisplay::prepareFramebufferStack(MTKView *view, MWKOpenGLContext *context) {
    framebufferWidth = view.drawableSize.width;
    framebufferHeight = view.drawableSize.height;
    
    {
        NSDictionary *pixelBufferAttributes = @{
            (NSString *)kCVPixelBufferPixelFormatTypeKey: @(kCVPixelFormatType_64RGBAHalf),
            (NSString *)kCVPixelBufferWidthKey: @(framebufferWidth),
            (NSString *)kCVPixelBufferHeightKey: @(framebufferHeight),
            (NSString *)kCVPixelBufferMetalCompatibilityKey: @(YES),
#if TARGET_OS_OSX
            (NSString *)kCVPixelBufferOpenGLCompatibilityKey: @(YES),
#else
            (NSString *)kCVPixelBufferOpenGLESCompatibilityKey: @(YES),
#endif
        };
        CVPixelBufferPoolRef _cvPixelBufferPool = nullptr;
        auto status = CVPixelBufferPoolCreate(kCFAllocatorDefault,
                                              nullptr,
                                              (__bridge CFDictionaryRef)pixelBufferAttributes,
                                              &_cvPixelBufferPool);
        if (status != kCVReturnSuccess) {
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN,
                                  boost::format("Cannot create pixel buffer pool (error = %d)") % status);
        }
        cvPixelBufferPool = CVPixelBufferPoolPtr::owned(_cvPixelBufferPool);
    }
    
    {
        CVMetalTextureCacheRef _cvMetalTextureCache = nullptr;
        auto status = CVMetalTextureCacheCreate(kCFAllocatorDefault,
                                                nullptr,
                                                view.device,
                                                nullptr,
                                                &_cvMetalTextureCache);
        if (status != kCVReturnSuccess) {
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN,
                                  boost::format("Cannot create Metal texture cache (error = %d)") % status);
        }
        cvMetalTextureCache = CVMetalTextureCachePtr::created(_cvMetalTextureCache);
    }
    
    {
#if TARGET_OS_OSX
        CVOpenGLTextureCacheRef _cvOpenGLTextureCache = nullptr;
        auto status = CVOpenGLTextureCacheCreate(kCFAllocatorDefault,
                                                 nullptr,
                                                 context.CGLContextObj,
                                                 context.pixelFormat.CGLPixelFormatObj,
                                                 nullptr,
                                                 &_cvOpenGLTextureCache);
#else
        CVOpenGLESTextureCacheRef _cvOpenGLTextureCache = nullptr;
        auto status = CVOpenGLESTextureCacheCreate(kCFAllocatorDefault,
                                                   nullptr,
                                                   context,
                                                   nullptr,
                                                   &_cvOpenGLTextureCache);
#endif
        if (status != kCVReturnSuccess) {
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN,
                                  boost::format("Cannot create OpenGL texture cache (error = %d)") % status);
        }
        cvOpenGLTextureCache = CVOpenGLTextureCachePtr::created(_cvOpenGLTextureCache);
    }
}


int AppleStimulusDisplay::createFramebuffer() {
    Framebuffer framebuffer;
    
    {
        CVPixelBufferRef _cvPixelBuffer = nullptr;
        auto status = CVPixelBufferPoolCreatePixelBuffer(kCFAllocatorDefault,
                                                         cvPixelBufferPool.get(),
                                                         &_cvPixelBuffer);
        if (status != kCVReturnSuccess) {
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN,
                                  boost::format("Cannot create pixel buffer (error = %d)") % status);
        }
        framebuffer.cvPixelBuffer = CVPixelBufferPtr::owned(_cvPixelBuffer);
    }
    
    {
        CVMetalTextureRef _cvMetalTexture = nullptr;
        auto status = CVMetalTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
                                                                cvMetalTextureCache.get(),
                                                                framebuffer.cvPixelBuffer.get(),
                                                                nullptr,
                                                                getMetalFramebufferTexturePixelFormat(),
                                                                framebufferWidth,
                                                                framebufferHeight,
                                                                0,
                                                                &_cvMetalTexture);
        if (status != kCVReturnSuccess) {
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN,
                                  boost::format("Cannot create Metal texture (error = %d)") % status);
        }
        framebuffer.cvMetalTexture = CVMetalTexturePtr::created(_cvMetalTexture);
    }
    
    {
#if TARGET_OS_OSX
        CVOpenGLTextureRef _cvOpenGLTexture = nullptr;
        auto status = CVOpenGLTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
                                                                 cvOpenGLTextureCache.get(),
                                                                 framebuffer.cvPixelBuffer.get(),
                                                                 nullptr,
                                                                 &_cvOpenGLTexture);
#else
        CVOpenGLESTextureRef _cvOpenGLTexture = nullptr;
        auto status = CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
                                                                   cvOpenGLTextureCache.get(),
                                                                   framebuffer.cvPixelBuffer.get(),
                                                                   nullptr,
                                                                   GL_TEXTURE_2D,
                                                                   GL_RGBA16F,
                                                                   framebufferWidth,
                                                                   framebufferHeight,
                                                                   GL_RGBA,
                                                                   GL_HALF_FLOAT,
                                                                   0,
                                                                   &_cvOpenGLTexture);
#endif
        if (status != kCVReturnSuccess) {
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN,
                                  boost::format("Cannot create OpenGL texture (error = %d)") % status);
        }
        framebuffer.cvOpenGLTexture = CVOpenGLTexturePtr::created(_cvOpenGLTexture);
    }
    
    {
        GLuint _glFramebuffer = 0;
        glGenFramebuffers(1, &_glFramebuffer);
        gl::FramebufferBinding<GL_DRAW_FRAMEBUFFER> framebufferBinding(_glFramebuffer);
        
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER,
                               GL_COLOR_ATTACHMENT0,
#if TARGET_OS_OSX
                               CVOpenGLTextureGetTarget(framebuffer.cvOpenGLTexture.get()),
                               CVOpenGLTextureGetName(framebuffer.cvOpenGLTexture.get()),
#else
                               CVOpenGLESTextureGetTarget(framebuffer.cvOpenGLTexture.get()),
                               CVOpenGLESTextureGetName(framebuffer.cvOpenGLTexture.get()),
#endif
                               0);
        
        if (GL_FRAMEBUFFER_COMPLETE != glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER)) {
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "OpenGL framebuffer setup failed");
        }
        
        framebuffer.glFramebuffer = _glFramebuffer;
    }
    
    int framebuffer_id = 0;
    auto lastIter = framebuffers.rbegin();
    if (lastIter != framebuffers.rend()) {
        framebuffer_id = lastIter->first + 1;
    }
    framebuffers.emplace(framebuffer_id, std::move(framebuffer));
    return framebuffer_id;
}


void AppleStimulusDisplay::pushFramebuffer(int framebuffer_id) {
    auto iter = framebuffers.find(framebuffer_id);
    if (iter == framebuffers.end()) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "Internal error: invalid framebuffer ID: %d", framebuffer_id);
        return;
    }
    framebufferStack.push_back(iter->first);
    bindFramebuffer(iter->second);
}


void AppleStimulusDisplay::bindCurrentFramebuffer() {
    if (framebufferStack.empty()) {
        currentFramebufferTexture = nil;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);  // Bind the default framebuffer
        return;
    }
    bindFramebuffer(framebuffers.find(framebufferStack.back())->second);
}


void AppleStimulusDisplay::bindFramebuffer(Framebuffer &framebuffer) {
    currentFramebufferTexture = CVMetalTextureGetTexture(framebuffer.cvMetalTexture.get());
    
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer.glFramebuffer);
    const GLenum drawBuffer = GL_COLOR_ATTACHMENT0;
    glDrawBuffers(1, &drawBuffer);
    glViewport(0, 0, framebufferWidth, framebufferHeight);
}


void AppleStimulusDisplay::popFramebuffer() {
    if (framebufferStack.empty()) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "Internal error: no framebuffer to pop");
        return;
    }
    framebufferStack.pop_back();
    bindCurrentFramebuffer();
}


void AppleStimulusDisplay::releaseFramebuffer(int framebuffer_id) {
    auto iter = framebuffers.find(framebuffer_id);
    if (iter == framebuffers.end()) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "Internal error: invalid framebuffer ID: %d", framebuffer_id);
        return;
    }
    if (std::find(framebufferStack.begin(), framebufferStack.end(), iter->first) != framebufferStack.end()) {
        merror(M_DISPLAY_MESSAGE_DOMAIN,
               "Internal error: framebuffer %d is in use and cannot be released",
               iter->first);
        return;
    }
    glDeleteFramebuffers(1, &(iter->second.glFramebuffer));
    framebuffers.erase(iter);
}


id<MTLTexture> AppleStimulusDisplay::getMetalFramebufferTexture(int framebuffer_id) const {
    auto iter = framebuffers.find(framebuffer_id);
    if (iter == framebuffers.end()) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "Internal error: invalid framebuffer ID: %d", framebuffer_id);
        return nil;
    }
    return CVMetalTextureGetTexture(iter->second.cvMetalTexture.get());
}


void AppleStimulusDisplay::configureCapture(const std::string &format, int heightPixels, const VariablePtr &enabled) {
    lock_guard lock(mutex);
    
    if (!mainView) {
        throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Cannot configure display capture before setting main context");
    }
    if (FrameCaptureManager::metalTexturePixelFormat != mainView.colorPixelFormat) {
        throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN,
                              boost::format("Internal error: main view has unexpected pixel format (%1%)")
                              % mainView.colorPixelFormat);
    }
    
    std::size_t bufferHeight = framebufferHeight;
    std::size_t bufferWidth = framebufferWidth;
    if (heightPixels > framebufferHeight) {
        mwarning(M_DISPLAY_MESSAGE_DOMAIN,
                 "Requested stimulus display capture buffer height (%d pixels) is larger than stimulus display height "
                 "(%lu pixels); stimulus display height will be used",
                 heightPixels,
                 framebufferHeight);
    } else if (heightPixels > 0) {
        bufferHeight = heightPixels;
        bufferWidth = bufferHeight * double(framebufferWidth) / double(framebufferHeight);
    }
    
    cf::StringPtr imageFileType;
    if (format == "JPEG") {
        imageFileType = cf::StringPtr::borrowed(kUTTypeJPEG);
    } else if (format == "PNG") {
        imageFileType = cf::StringPtr::borrowed(kUTTypePNG);
    } else {
        throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN,
                              boost::format("Invalid stimulus display capture format: \"%1%\"") % format);
    }
    
    auto imageFileColorSpace = CGColorSpacePtr::created(getUseColorManagement() ?
                                                        CGColorSpaceCreateWithName(kCGColorSpaceSRGB) :
                                                        CGColorSpaceCreateDeviceRGB());
    
    captureManager = std::make_unique<FrameCaptureManager>(bufferWidth,
                                                           bufferHeight,
                                                           imageFileType,
                                                           imageFileColorSpace,
                                                           enabled,
                                                           device);
}


void AppleStimulusDisplay::captureCurrentFrame() {
    const auto captureOutputTimeUS = getEventTimeForCurrentOutputTime();
    
    CVPixelBufferPtr cvPixelBuffer;
    CVMetalTexturePtr cvMetalTexture;
    if (!captureManager->createCaptureBuffer(cvPixelBuffer, cvMetalTexture)) {
        merror(M_DISPLAY_MESSAGE_DOMAIN,
               "Cannot capture stimulus display frame for output time %lld",
               captureOutputTimeUS);
        return;
    }
    
    MTLRenderPassDescriptor *renderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
    renderPassDescriptor.colorAttachments[0].texture = CVMetalTextureGetTexture(cvMetalTexture.get());
    [mainViewDelegate renderWithRenderPassDescriptor:renderPassDescriptor];
    
    boost::weak_ptr<AppleStimulusDisplay> weakThis(boost::dynamic_pointer_cast<AppleStimulusDisplay>(shared_from_this()));
    auto completedHandler = [weakThis, cvPixelBuffer, captureOutputTimeUS](id<MTLCommandBuffer> commandBuffer) {
        if (commandBuffer.status == MTLCommandBufferStatusCompleted) {
            // Create the image file on a global queue, so that we don't block a thread doing
            // graphics-processing work.  Specify QOS_CLASS_USER_INITIATED to ensure that the
            // task runs at relatively high priority.
            dispatch_async(dispatch_get_global_queue(QOS_CLASS_USER_INITIATED, 0),
                           [weakThis, cvPixelBuffer, captureOutputTimeUS]() {
                if (auto sharedThis = weakThis.lock()) {
                    std::string imageFile;
                    if (!sharedThis->captureManager->convertCaptureBufferToImageFile(cvPixelBuffer, imageFile)) {
                        merror(M_DISPLAY_MESSAGE_DOMAIN,
                               "Cannot capture stimulus display frame for output time %lld",
                               captureOutputTimeUS);
                    } else {
                        // Acquire a local strong reference to stimDisplayCapture to guard against the situation
                        // where the experiment is being unloaded while this code is executing
                        if (auto sdc = stimDisplayCapture) {
                            Datum value(std::move(imageFile));
                            // JPEG and PNG are compressed formats, so we're likely to spend many CPU cycles for
                            // little reduction in event file size if we try to compress the image data again
                            value.setCompressible(false);
                            sdc->setValue(std::move(value), captureOutputTimeUS);
                        }
                    }
                }
            });
        }
    };
    
    [currentCommandBuffer addCompletedHandler:completedHandler];
}


AppleStimulusDisplay::FrameCaptureManager::FrameCaptureManager(std::size_t bufferWidth,
                                                               std::size_t bufferHeight,
                                                               const cf::StringPtr &imageFileType,
                                                               const CGColorSpacePtr &imageFileColorSpace,
                                                               const boost::shared_ptr<Variable> &enabled,
                                                               id<MTLDevice> metalDevice) :
    bufferWidth(bufferWidth),
    bufferHeight(bufferHeight),
    imageFileType(imageFileType),
    imageFileColorSpace(imageFileColorSpace),
    enabled(enabled)
{
    @autoreleasepool {
        NSDictionary *pixelBufferAttributes = @{
            (NSString *)kCVPixelBufferPixelFormatTypeKey: @(pixelBufferPixelFormat),
            (NSString *)kCVPixelBufferWidthKey: @(bufferWidth),
            (NSString *)kCVPixelBufferHeightKey: @(bufferHeight),
            (NSString *)kCVPixelBufferCGImageCompatibilityKey: @(YES),
            (NSString *)kCVPixelBufferMetalCompatibilityKey: @(YES)
        };
        CVPixelBufferPoolRef _pixelBufferPool = nullptr;
        auto status = CVPixelBufferPoolCreate(kCFAllocatorDefault,
                                              nullptr,
                                              (__bridge CFDictionaryRef)pixelBufferAttributes,
                                              &_pixelBufferPool);
        if (status != kCVReturnSuccess) {
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN,
                                  boost::format("Cannot create pixel buffer pool (error = %d)") % status);
        }
        pixelBufferPool = CVPixelBufferPoolPtr::owned(_pixelBufferPool);
        
        NSDictionary *_pixelBufferAuxAttributes = @{
            // Don't allow more than 5 in-flight pixel buffers
            (NSString *)kCVPixelBufferPoolAllocationThresholdKey: @(5)
        };
        pixelBufferAuxAttributes = cf::DictionaryPtr::borrowed((__bridge CFDictionaryRef)_pixelBufferAuxAttributes);
    }
    
    {
        CVMetalTextureCacheRef _metalTextureCache = nullptr;
        auto status = CVMetalTextureCacheCreate(kCFAllocatorDefault,
                                                nullptr,
                                                metalDevice,
                                                nullptr,
                                                &_metalTextureCache);
        if (status != kCVReturnSuccess) {
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN,
                                  boost::format("Cannot create Metal texture cache (error = %d)") % status);
        }
        metalTextureCache = CVMetalTextureCachePtr::created(_metalTextureCache);
    }
}


bool AppleStimulusDisplay::FrameCaptureManager::createCaptureBuffer(CVPixelBufferPtr &pixelBuffer,
                                                                    CVMetalTexturePtr &metalTexture)
{
    {
        CVPixelBufferRef _pixelBuffer = nullptr;
        auto status = CVPixelBufferPoolCreatePixelBufferWithAuxAttributes(kCFAllocatorDefault,
                                                                          pixelBufferPool.get(),
                                                                          pixelBufferAuxAttributes.get(),
                                                                          &_pixelBuffer);
        if (status != kCVReturnSuccess) {
            if (status == kCVReturnWouldExceedAllocationThreshold) {
                merror(M_DISPLAY_MESSAGE_DOMAIN, "Cannot create pixel buffer: too many buffers already in use");
            } else {
                merror(M_DISPLAY_MESSAGE_DOMAIN, "Cannot create pixel buffer (error = %d)", status);
            }
            return false;
        }
        pixelBuffer = CVPixelBufferPtr::owned(_pixelBuffer);
    }
    
    {
        CVMetalTextureRef _metalTexture = nullptr;
        auto status = CVMetalTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
                                                                metalTextureCache.get(),
                                                                pixelBuffer.get(),
                                                                nullptr,
                                                                metalTexturePixelFormat,
                                                                bufferWidth,
                                                                bufferHeight,
                                                                0,
                                                                &_metalTexture);
        if (status != kCVReturnSuccess) {
            merror(M_DISPLAY_MESSAGE_DOMAIN, "Cannot create Metal texture (error = %d)", status);
            return false;
        }
        metalTexture = CVMetalTexturePtr::owned(_metalTexture);
    }
    
    return true;
}


bool AppleStimulusDisplay::FrameCaptureManager::convertCaptureBufferToImageFile(const CVPixelBufferPtr &pixelBuffer,
                                                                                std::string &imageFile) const
{
    CGImagePtr image;
    {
        CGImageRef _image = nullptr;
        auto status = VTCreateCGImageFromCVPixelBuffer(pixelBuffer.get(), nullptr, &_image);
        if (status != noErr) {
            merror(M_DISPLAY_MESSAGE_DOMAIN, "Cannot create image from pixel buffer (error = %d)", status);
            return false;
        }
        image = CGImagePtr::owned(_image);
    }
    image = CGImagePtr::created(CGImageCreateCopyWithColorSpace(image.get(), imageFileColorSpace.get()));
    
    auto imageFileData = cf::ObjectPtr<CFMutableDataRef>::created(CFDataCreateMutable(kCFAllocatorDefault, 0));
    auto imageDest = cf::ObjectPtr<CGImageDestinationRef>::created(CGImageDestinationCreateWithData(imageFileData.get(),
                                                                                                    imageFileType.get(),
                                                                                                    1,
                                                                                                    nullptr));
    
    // NOTE: By setting the property kCGImagePropertyHasAlpha to NO when adding the image, we can reduce
    // the size of PNG files.  However, doing so greatly increases the already-high CPU load (by a factor
    // of 3 in some tests).  On balance, hammering the CPU to get slightly-smaller image files doesn't
    // seem like a good tradeoff.
    CGImageDestinationAddImage(imageDest.get(), image.get(), nullptr);
    
    if (!CGImageDestinationFinalize(imageDest.get())) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "Cannot create image file from pixel buffer image");
        return false;
    }
    
    imageFile.assign(reinterpret_cast<const char *>(CFDataGetBytePtr(imageFileData.get())),
                     CFDataGetLength(imageFileData.get()));
    
    return true;
}


END_NAMESPACE_MW
