//
//  AppleStimulusDisplay.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 12/14/20.
//

#include "AppleStimulusDisplay.hpp"

#include "OpenGLUtilities.hpp"


NS_ASSUME_NONNULL_BEGIN


@interface MWKStimulusDisplayViewDelegate : NSObject <MTKViewDelegate>

@property(nonatomic, strong) id<MTLCommandQueue> commandQueue;
@property(nonatomic, strong) id<MTLTexture> texture;

- (BOOL)prepareWithMTKView:(MTKView *)view
        useColorManagement:(BOOL)useColorManagement
                     error:(__autoreleasing NSError **)error;

@end


NS_ASSUME_NONNULL_END


@implementation MWKStimulusDisplayViewDelegate {
    id<MTLRenderPipelineState> _pipelineState;
}


- (BOOL)prepareWithMTKView:(MTKView *)view useColorManagement:(BOOL)useColorManagement error:(NSError **)error {
    id<MTLLibrary> library = [view.device newDefaultLibraryWithBundle:[NSBundle bundleForClass:[self class]]
                                                                error:error];
    if (!library) {
        return NO;
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
        return NO;
    }
    
    MTLRenderPipelineDescriptor *pipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    pipelineStateDescriptor.vertexFunction = vertexFunction;
    pipelineStateDescriptor.fragmentFunction = fragmentFunction;
    pipelineStateDescriptor.colorAttachments[0].pixelFormat = view.colorPixelFormat;
    
    _pipelineState = [view.device newRenderPipelineStateWithDescriptor:pipelineStateDescriptor error:error];
    if (!_pipelineState) {
        return NO;
    }
    
    return YES;
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
    contextManager(boost::dynamic_pointer_cast<AppleOpenGLContextManager>(opengl_context_manager)),
    commandQueue(nil),
    mainView(nil),
    mirrorView(nil),
    mainViewDelegate(nil),
    mirrorViewDelegate(nil),
    framebufferWidth(0),
    framebufferHeight(0)
{ }


AppleStimulusDisplay::~AppleStimulusDisplay() {
    @autoreleasepool {
        mirrorViewDelegate = nil;
        mainViewDelegate = nil;
        mirrorView = nil;
        mainView = nil;
        commandQueue = nil;
    }
}


void AppleStimulusDisplay::prepareContext(int context_id) {
    @autoreleasepool {
        opengl_context_manager->prepareContext(context_id, useColorManagement);
        
        MTKView *view = contextManager->getView(context_id);
        view.paused = YES;
        view.enableSetNeedsDisplay = NO;
        
        MWKStimulusDisplayViewDelegate *delegate = [[MWKStimulusDisplayViewDelegate alloc] init];
        NSError *error = nil;
        if (![delegate prepareWithMTKView:view useColorManagement:useColorManagement error:&error]) {
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN,
                                  "Cannot prepare Metal view delegate",
                                  error.localizedDescription.UTF8String);
        }
        
        if (context_id == main_context_id) {
            prepareFramebufferStack(view, contextManager->getContext(context_id));
            commandQueue = [view.device newCommandQueue];
            mainView = view;
            mainViewDelegate = delegate;
        } else if (context_id == mirror_context_id) {
            mirrorView = view;
            mirrorViewDelegate = delegate;
        }
        
        delegate.commandQueue = commandQueue;
        view.delegate = delegate;
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
        // If the framebuffer stack is empty, then we're about to draw to the screen.
        // Bind the default framebuffer, then call glFlush to ensure that any pending
        // OpenGL commands are committed before we proceed.
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glFlush();
        return;
    }
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebufferStack.back().glFramebuffer);
    const GLenum drawBuffer = GL_COLOR_ATTACHMENT0;
    glDrawBuffers(1, &drawBuffer);
    glViewport(0, 0, framebufferWidth, framebufferHeight);
}


void AppleStimulusDisplay::presentFramebuffer(int framebuffer_id, int dst_context_id) {
    @autoreleasepool {
        auto iter = framebuffers.find(framebuffer_id);
        if (iter == framebuffers.end()) {
            merror(M_DISPLAY_MESSAGE_DOMAIN, "Internal error: invalid framebuffer ID: %d", framebuffer_id);
            return;
        }
        
        MWKStimulusDisplayViewDelegate *delegate = nil;
        MTKView *view = nil;
        
        if (dst_context_id == main_context_id) {
            delegate = mainViewDelegate;
            view = mainView;
        } else if (dst_context_id == mirror_context_id) {
            delegate = mirrorViewDelegate;
            view = mirrorView;
        }
        
        delegate.texture = CVMetalTextureGetTexture(iter->second.cvMetalTexture.get());
        [view draw];
    }
}


id<MTLTexture> AppleStimulusDisplay::getCurrentMetalFramebufferTexture() const {
    if (framebufferStack.empty()) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "Internal error: no current Metal framebuffer texture");
        return nil;
    }
    return CVMetalTextureGetTexture(framebufferStack.back().cvMetalTexture.get());
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
