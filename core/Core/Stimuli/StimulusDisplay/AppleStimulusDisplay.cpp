//
//  AppleStimulusDisplay.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 12/14/20.
//

#include "AppleStimulusDisplay.hpp"

#include "AAPLMathUtilities.h"
#include "OpenGLUtilities.hpp"
#include "Stimulus.h"

using namespace mw::aapl_math_utilities;


@interface MWKStimulusDisplayViewDelegate : NSObject <MTKViewDelegate>

@property(nonatomic, readonly) id<MTLCommandQueue> commandQueue;
@property(nonatomic, strong) id<MTLTexture> texture;

+ (instancetype)delegateWithMTKView:(MTKView *)view
                 useColorManagement:(BOOL)useColorManagement
                              error:(__autoreleasing NSError **)error;

- (instancetype)initWithCommandQueue:(id<MTLCommandQueue>)commandQueue
                       pipelineState:(id<MTLRenderPipelineState>)pipelineState;

@end


@implementation MWKStimulusDisplayViewDelegate {
    id<MTLRenderPipelineState> _pipelineState;
}


+ (instancetype)delegateWithMTKView:(MTKView *)view
                 useColorManagement:(BOOL)useColorManagement
                              error:(__autoreleasing NSError **)error
{
    id<MTLLibrary> library = [view.device newDefaultLibraryWithBundle:MWORKS_GET_CURRENT_BUNDLE() error:error];
    if (!library) {
        return nil;
    }
    
    MTLFunctionConstantValues *functionConstantValues = [[MTLFunctionConstantValues alloc] init];
    const bool convertToSRGB = useColorManagement;
    [functionConstantValues setConstantValue:&convertToSRGB type:MTLDataTypeBool atIndex:0];
    
    id<MTLFunction> vertexFunction = [library newFunctionWithName:@"MWKStimulusDisplayViewDelegate_vertexShader"];
    
    id<MTLFunction> fragmentFunction = [library newFunctionWithName:@"MWKStimulusDisplayViewDelegate_fragmentShader"
                                                     constantValues:functionConstantValues
                                                              error:error];
    if (!fragmentFunction) {
        return nil;
    }
    
    MTLRenderPipelineDescriptor *pipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    pipelineStateDescriptor.vertexFunction = vertexFunction;
    pipelineStateDescriptor.fragmentFunction = fragmentFunction;
    pipelineStateDescriptor.colorAttachments[0].pixelFormat = view.colorPixelFormat;
    
    id<MTLRenderPipelineState> pipelineState = [view.device newRenderPipelineStateWithDescriptor:pipelineStateDescriptor
                                                                                           error:error];
    if (!pipelineState) {
        return nil;
    }
    
    return [[self alloc] initWithCommandQueue:[view.device newCommandQueue] pipelineState:pipelineState];
}


- (instancetype)initWithCommandQueue:(id<MTLCommandQueue>)commandQueue
                       pipelineState:(id<MTLRenderPipelineState>)pipelineState
{
    self = [super init];
    if (self) {
        _commandQueue = commandQueue;
        _pipelineState = pipelineState;
    }
    return self;
}


- (void)mtkView:(MTKView *)view drawableSizeWillChange:(CGSize)size {
    // Stimulus display views aren't resizeable, so this should never be called
}


- (void)drawInMTKView:(MTKView *)view {
    MTLRenderPassDescriptor *renderPassDescriptor = view.currentRenderPassDescriptor;
    if (renderPassDescriptor) {
        id<MTLCommandBuffer> commandBuffer = [self.commandQueue commandBuffer];
        id<MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
        
        [renderEncoder setRenderPipelineState:_pipelineState];
        [renderEncoder setFragmentTexture:self.texture atIndex:0];
        
        [renderEncoder drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];
        
        [renderEncoder endEncoding];
        [commandBuffer presentDrawable:view.currentDrawable];
        [commandBuffer commit];
    }
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
                MTLRenderPassDescriptor *renderPassDescriptor = createMetalRenderPassDescriptor(MTLLoadActionClear);
                renderPassDescriptor.colorAttachments[0].clearColor = clearColor;
                id<MTLRenderCommandEncoder> renderEncoder = [currentCommandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
                [renderEncoder endEncoding];
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
            
            [currentCommandBuffer commit];
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
                                                nil,
                                                view.device,
                                                nil,
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
                                                 nil,
                                                 context.CGLContextObj,
                                                 context.pixelFormat.CGLPixelFormatObj,
                                                 nil,
                                                 &_cvOpenGLTextureCache);
#else
        CVOpenGLESTextureCacheRef _cvOpenGLTextureCache = nullptr;
        auto status = CVOpenGLESTextureCacheCreate(kCFAllocatorDefault,
                                                   nil,
                                                   context,
                                                   nil,
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
                                                                nil,
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
                                                                 nil,
                                                                 &_cvOpenGLTexture);
#else
        CVOpenGLESTextureRef _cvOpenGLTexture = nullptr;
        auto status = CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
                                                                   cvOpenGLTextureCache.get(),
                                                                   framebuffer.cvPixelBuffer.get(),
                                                                   nil,
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
            throw SimpleException("OpenGL framebuffer setup failed");
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


END_NAMESPACE_MW
