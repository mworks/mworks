//
//  AppleStimulusDisplay.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 12/14/20.
//

#include "AppleStimulusDisplay.hpp"

#include "OpenGLUtilities.hpp"
#include "Stimulus.h"


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
    id<MTLLibrary> library = [view.device newDefaultLibraryWithBundle:[NSBundle bundleForClass:self] error:error];
    if (!library) {
        return nil;
    }
    
    MTLFunctionConstantValues *functionConstantValues = [[MTLFunctionConstantValues alloc] init];
    const bool convertToSRGB = useColorManagement;
    [functionConstantValues setConstantValue:&convertToSRGB
                                        type:MTLDataTypeBool
                                    withName:@"MWKStimulusDisplayViewDelegate_convertToSRGB"];
    
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
    framebuffer_id(-1)
{ }


AppleStimulusDisplay::~AppleStimulusDisplay() {
    @autoreleasepool {
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
            pushFramebuffer(framebuffer_id);
            
#if !MWORKS_OPENGL_ES
            // This has no effect on non-sRGB framebuffers, so we can enable it unconditionally
            gl::Enabled<GL_FRAMEBUFFER_SRGB> framebufferSRGBEnabled;
#endif
            
            glDisable(GL_BLEND);
            glDisable(GL_DITHER);
            
            double backgroundRed, backgroundGreen, backgroundBlue, backgroundAlpha;
            getBackgroundColor(backgroundRed, backgroundGreen, backgroundBlue, backgroundAlpha);
            glClearColor(backgroundRed, backgroundGreen, backgroundBlue, backgroundAlpha);
            glClear(GL_COLOR_BUFFER_BIT);
            
            auto sharedThis = shared_from_this();
            for (auto &stim : stimsToDraw) {
                stim->draw(sharedThis);
            }
            
            popFramebuffer();
            glFlush();  // Commit pending OpenGL commands
            
            if (mirrorView) {
                dispatch_async(dispatch_get_main_queue(), ^{
#if TARGET_OS_OSX
                    mirrorView.needsDisplay = YES;
#else
                    [mirrorView setNeedsDisplay];
#endif
                });
            }
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
                                                                MTLPixelFormatRGBA16Float,
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
    framebufferStack.emplace_back(iter->second);
    bindCurrentFramebuffer();
}


void AppleStimulusDisplay::bindCurrentFramebuffer() {
    if (framebufferStack.empty()) {
        // Bind the default framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebufferStack.back().glFramebuffer);
    const GLenum drawBuffer = GL_COLOR_ATTACHMENT0;
    glDrawBuffers(1, &drawBuffer);
    glViewport(0, 0, framebufferWidth, framebufferHeight);
}


id<MTLTexture> AppleStimulusDisplay::getMetalFramebufferTexture(int framebuffer_id) const {
    auto iter = framebuffers.find(framebuffer_id);
    if (iter == framebuffers.end()) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "Internal error: invalid framebuffer ID: %d", framebuffer_id);
        return nil;
    }
    return CVMetalTextureGetTexture(iter->second.cvMetalTexture.get());
}


id<MTLTexture> AppleStimulusDisplay::getCurrentMetalFramebufferTexture() const {
    if (framebufferStack.empty()) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "Internal error: no current Metal framebuffer texture");
        return nil;
    }
    return CVMetalTextureGetTexture(framebufferStack.back().cvMetalTexture.get());
}


MTLRenderPassDescriptor * AppleStimulusDisplay::createMetalRenderPassDescriptor() const {
    id<MTLTexture> texture = getCurrentMetalFramebufferTexture();
    if (!texture) {
        return nil;
    }
    MTLRenderPassDescriptor *renderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
    renderPassDescriptor.colorAttachments[0].texture = texture;
    renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionLoad;
    renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
    return renderPassDescriptor;
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
    framebuffers.erase(iter);
}


END_NAMESPACE_MW
