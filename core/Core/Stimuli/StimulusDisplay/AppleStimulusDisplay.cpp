//
//  AppleStimulusDisplay.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 12/14/20.
//

#include "AppleStimulusDisplay.hpp"

#include <CoreServices/CoreServices.h>
#include <UniformTypeIdentifiers/UniformTypeIdentifiers.h>
#include <VideoToolbox/VideoToolbox.h>

#include "AAPLMathUtilities.h"
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


AppleStimulusDisplay::AppleStimulusDisplay(const Configuration &config) :
    StimulusDisplay(config),
    device(nil),
    commandQueue(nil),
    mainView(nil),
    mirrorView(nil),
    mainViewDelegate(nil),
    mirrorViewDelegate(nil),
    defaultFramebufferWidth(0),
    defaultFramebufferHeight(0),
    defaultFramebufferID(-1),
    currentFramebufferTexture(nil),
    currentCommandBuffer(nil),
    defaultViewport(createViewport(0.0, 0.0, 0.0, 0.0)),
    defaultProjectionMatrix(createProjectionMatrix(-1.0, 1.0, -1.0, 1.0)),
    pixelsPerDisplayUnit(0.0)
{ }


AppleStimulusDisplay::~AppleStimulusDisplay() {
    @autoreleasepool {
        currentCommandBuffer = nil;
        currentFramebufferTexture = nil;
        framebuffers.clear();  // Release framebuffers in scope of autorelease pool
        mirrorViewDelegate = nil;
        mainViewDelegate = nil;
        mirrorView = nil;
        mainView = nil;
        commandQueue = nil;
        device = nil;
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
            
            defaultFramebufferWidth = view.drawableSize.width;
            defaultFramebufferHeight = view.drawableSize.height;
            defaultFramebufferID = createFramebuffer();
            defaultViewport = createViewport(0, 0, defaultFramebufferWidth, defaultFramebufferHeight);
            
            double left, right, bottom, top;
            getDisplayBounds(left, right, bottom, top);
            defaultProjectionMatrix = createProjectionMatrix(left, right, bottom, top);
            
            //
            // Evaluate pixels per display unit in both the horizontal and vertical directions,
            // and store the larger of the two.  In the case where the configured aspect ratio
            // of the display is different from the drawable's aspect ratio, this will ensure
            // that textures sized using pixelsPerDisplayUnit will have at least one texel per
            // pixel even under arbitrary rotation.
            //
            pixelsPerDisplayUnit = std::max(double(defaultFramebufferWidth) / (right - left),
                                            double(defaultFramebufferHeight) / (top - bottom));
        }
        
        delegate.texture = getMetalFramebufferTexture(defaultFramebufferID);
        view.delegate = delegate;
    }
}


void AppleStimulusDisplay::renderDisplay(bool needDraw, const std::vector<boost::shared_ptr<Stimulus>> &stimsToDraw) {
    @autoreleasepool {
        if (needDraw) {
            pushFramebuffer(defaultFramebufferID);
            
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
                stim->draw(sharedThis);
            }
            
            // Pass the current command buffer on to the main view delegate, which will commit and
            // release the buffer after drawing to the display
            mainViewDelegate.currentCommandBuffer = currentCommandBuffer;
            
            if (mirrorView) {
                //
                // Wait until all main view rendering commands have been scheduled on the GPU
                // before triggering a mirror window update.  This avoids the situation where,
                // when display updates are stopped, the mirror window remains frozen on the
                // last frame *before* the display was cleared.
                //
                // Also, since the mirror window update trigger is asynchronous, it's possible
                // that this AppleStimulusDisplay instance will be deallocated before it executes.
                // (This is most likely to happen when experiment loading fails, and the stimulus
                // display is destroyed almost immediately after being created.)  To avoid
                // sending messages to a deallocated instance, capture mirrorView as an ObjC
                // weak reference.
                //
                MTKView * __weak weakMirrorView = mirrorView;
                [currentCommandBuffer addScheduledHandler:^(id<MTLCommandBuffer> commandBuffer) {
                    dispatch_async(dispatch_get_main_queue(), ^{
#if TARGET_OS_OSX
                        weakMirrorView.needsDisplay = YES;
#else
                        [weakMirrorView setNeedsDisplay];
#endif
                    });
                }];
            }
            
            if (captureManager && captureManager->isCaptureEnabled()) {
                captureCurrentFrame();
            }
            
            currentCommandBuffer = nil;
            
            popFramebuffer();
        }
        
        [mainView draw];
    }
}


int AppleStimulusDisplay::createFramebuffer(std::size_t width, std::size_t height) {
    @autoreleasepool {
        auto textureDescriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:getMetalFramebufferTexturePixelFormat()
                                                                                    width:width
                                                                                   height:height
                                                                                mipmapped:NO];
        textureDescriptor.storageMode = MTLStorageModePrivate;
        textureDescriptor.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
        
        Framebuffer framebuffer;
        framebuffer.texture = [device newTextureWithDescriptor:textureDescriptor];
        
        int framebufferID = 0;
        auto lastIter = framebuffers.rbegin();
        if (lastIter != framebuffers.rend()) {
            framebufferID = lastIter->first + 1;
        }
        framebuffers.emplace(framebufferID, std::move(framebuffer));
        return framebufferID;
    }
}


void AppleStimulusDisplay::pushFramebuffer(int framebufferID) {
    @autoreleasepool {
        auto iter = framebuffers.find(framebufferID);
        if (iter == framebuffers.end()) {
            merror(M_DISPLAY_MESSAGE_DOMAIN, "Internal error: invalid framebuffer ID: %d", framebufferID);
            return;
        }
        framebufferStack.push_back(iter->first);
        currentFramebufferTexture = iter->second.texture;
    }
}


void AppleStimulusDisplay::popFramebuffer() {
    @autoreleasepool {
        if (framebufferStack.empty()) {
            merror(M_DISPLAY_MESSAGE_DOMAIN, "Internal error: no framebuffer to pop");
            return;
        }
        framebufferStack.pop_back();
        if (framebufferStack.empty()) {
            currentFramebufferTexture = nil;
        } else {
            currentFramebufferTexture = framebuffers.find(framebufferStack.back())->second.texture;
        }
    }
}


void AppleStimulusDisplay::releaseFramebuffer(int framebufferID) {
    @autoreleasepool {
        auto iter = framebuffers.find(framebufferID);
        if (iter == framebuffers.end()) {
            merror(M_DISPLAY_MESSAGE_DOMAIN, "Internal error: invalid framebuffer ID: %d", framebufferID);
            return;
        }
        if (std::find(framebufferStack.begin(), framebufferStack.end(), iter->first) != framebufferStack.end()) {
            merror(M_DISPLAY_MESSAGE_DOMAIN,
                   "Internal error: framebuffer %d is in use and cannot be released",
                   iter->first);
            return;
        }
        framebuffers.erase(iter);
    }
}


id<MTLTexture> AppleStimulusDisplay::getMetalFramebufferTexture(int framebufferID) const {
    auto iter = framebuffers.find(framebufferID);
    if (iter == framebuffers.end()) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "Internal error: invalid framebuffer ID: %d", framebufferID);
        return nil;
    }
    return iter->second.texture;
}


MTLViewport AppleStimulusDisplay::createViewport(double originX, double originY, double width, double height) {
    return MTLViewport{ originX, originY, width, height, 0.0, 1.0 };
}


void AppleStimulusDisplay::pushMetalViewport(double originX, double originY, double width, double height) {
    viewportStack.emplace_back(createViewport(originX, originY, width, height));
}


void AppleStimulusDisplay::popMetalViewport() {
    if (viewportStack.empty()) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "Internal error: no Metal viewport to pop");
        return;
    }
    viewportStack.pop_back();
}


simd::float4x4 AppleStimulusDisplay::createProjectionMatrix(double left, double right, double bottom, double top) {
    //
    // Metal uses different normalized device coordinates than OpenGL, so Metal-based stimuli need to use a
    // different projection matrix than OpenGL-based ones, as explained at
    // https://metashapes.com/blog/opengl-metal-projection-matrix-problem/
    //
    return matrix_ortho_right_hand(left, right, bottom, top, -1.0, 1.0);
}


void AppleStimulusDisplay::pushMetalProjectionMatrix(double left, double right, double bottom, double top) {
    projectionMatrixStack.emplace_back(createProjectionMatrix(left, right, bottom, top));
}


void AppleStimulusDisplay::popMetalProjectionMatrix() {
    if (projectionMatrixStack.empty()) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "Internal error: no Metal projection matrix to pop");
        return;
    }
    projectionMatrixStack.pop_back();
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
    
    std::size_t bufferHeight = defaultFramebufferHeight;
    std::size_t bufferWidth = defaultFramebufferWidth;
    if (heightPixels > defaultFramebufferHeight) {
        mwarning(M_DISPLAY_MESSAGE_DOMAIN,
                 "Requested stimulus display capture buffer height (%d pixels) is larger than stimulus display height "
                 "(%lu pixels); stimulus display height will be used",
                 heightPixels,
                 defaultFramebufferHeight);
    } else if (heightPixels > 0) {
        bufferHeight = heightPixels;
        bufferWidth = bufferHeight * double(defaultFramebufferWidth) / double(defaultFramebufferHeight);
    }
    
    cf::StringPtr imageFileType;
    if (format == "JPEG") {
        imageFileType = cf::StringPtr::borrowed((__bridge CFStringRef)(UTTypeJPEG.identifier));
    } else if (format == "PNG") {
        imageFileType = cf::StringPtr::borrowed((__bridge CFStringRef)(UTTypePNG.identifier));
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
                        Datum value(std::move(imageFile));
                        // JPEG and PNG are compressed formats, so we're likely to spend many CPU cycles for
                        // little reduction in event file size if we try to compress the image data again
                        value.setCompressible(false);
                        sharedThis->getCaptureVar()->setValue(std::move(value), captureOutputTimeUS);
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
