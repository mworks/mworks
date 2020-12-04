//
//  AppleOpenGLContextManager.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 3/27/17.
//
//

#include "AppleOpenGLContextManager.hpp"

#include "OpenGLUtilities.hpp"

#include <Metal/Metal.h>
#include <simd/simd.h>


@implementation MWKOpenGLContext {
    mw::OpenGLContextLock::unique_lock::mutex_type mutex;
}


- (mw::OpenGLContextLock)lockContext {
    return mw::OpenGLContextLock(mw::OpenGLContextLock::unique_lock(mutex));
}


@end


namespace {
    const std::string librarySource
    (R"(
     using namespace metal;
     
     typedef struct {
         float4 clipSpacePosition [[position]];
         float2 textureCoordinate;
     } RasterizerData;
     
     vertex RasterizerData
     vertexShader(uint vertexID [[vertex_id]],
                  constant float2 *vertexPositions [[buffer(0)]],
                  constant float2 *texCoords [[buffer(1)]])
     {
         RasterizerData out;
         
         out.clipSpacePosition.xy = vertexPositions[vertexID];
         out.clipSpacePosition.z = 0.0;
         out.clipSpacePosition.w = 1.0;
         
         out.textureCoordinate = texCoords[vertexID];
         
         return out;
     }
     
     float3
     linearToSRGB(float3 linear)
     {
         float3 srgb = mix(12.92 * linear,
                           1.055 * pow(linear, 1.0/2.4) - 0.055,
                           float3(linear >= 0.0031308));
         // To exactly replicate the linear-to-sRGB conversion performed by the GPU, we must first map the
         // floating-point sRGB color components we just computed back to integers in the range [0, 255],
         // and then re-normalize them to [0.0, 1.0].  Counterintuitively, this rounding process results in
         // better-looking output, with less visible banding in grayscale gradients, than using non-rounded
         // values.
         return round(srgb * 255.0) / 255.0;
     }
     
     constant bool convertToSRGB [[function_constant(0)]];
     
     fragment float4
     fragmentShader(RasterizerData in [[stage_in]],
                    texture2d<float> colorTexture [[texture(0)]])
     {
         constexpr sampler textureSampler (mag_filter::linear, min_filter::linear);
         float4 colorSample = colorTexture.sample(textureSampler, in.textureCoordinate);
         if (convertToSRGB) {
             colorSample.rgb = linearToSRGB(colorSample.rgb);
         }
         return colorSample;
     }
     )");
    
    
    constexpr std::size_t numVertices = 4;
    
    
    const simd::float2 vertexPositions[numVertices] =
    {
        { -1.0f, -1.0f },
        {  1.0f, -1.0f },
        { -1.0f,  1.0f },
        {  1.0f,  1.0f },
    };
    
    
    const simd::float2 texCoords[numVertices] =
    {
        { 0.0f, 0.0f },
        { 1.0f, 0.0f },
        { 0.0f, 1.0f },
        { 1.0f, 1.0f },
    };
}


@interface MWKMetalView ()

- (void)drawTexture:(id<MTLTexture>)texture;

@end


@implementation MWKMetalView {
    id<MTLRenderPipelineState> _pipelineState;
    id<MTLBuffer> _vertexPositionBuffer;
    id<MTLBuffer> _texCoordsBuffer;
    __unsafe_unretained id<MTLTexture> _texture;  // NOT owned
}


- (instancetype)initWithFrame:(CGRect)frameRect device:(id<MTLDevice>)device {
    self = [super initWithFrame:frameRect device:device];
    if (self) {
        self.paused = YES;
        self.enableSetNeedsDisplay = NO;
        _commandQueue = [self.device newCommandQueue];
    }
    return self;
}


- (BOOL)prepareUsingColorManagement:(BOOL)useColorManagement error:(NSError **)error {
    id<MTLLibrary> library = [self.device newLibraryWithSource:@(librarySource.c_str())
                                                       options:nil
                                                         error:error];
    if (!library) {
        return NO;
    }
    
    MTLFunctionConstantValues *functionConstantValues = [[MTLFunctionConstantValues alloc] init];
    const bool convertToSRGB = useColorManagement;
    [functionConstantValues setConstantValue:&convertToSRGB type:MTLDataTypeBool atIndex:0];
    
    id<MTLFunction> vertexFunction = [library newFunctionWithName:@"vertexShader"];
    
    id<MTLFunction> fragmentFunction = [library newFunctionWithName:@"fragmentShader"
                                                     constantValues:functionConstantValues
                                                              error:error];
    if (!fragmentFunction) {
        return NO;
    }
    
    MTLRenderPipelineDescriptor *pipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    pipelineStateDescriptor.vertexFunction = vertexFunction;
    pipelineStateDescriptor.fragmentFunction = fragmentFunction;
    pipelineStateDescriptor.colorAttachments[0].pixelFormat = self.colorPixelFormat;
    
    _pipelineState = [self.device newRenderPipelineStateWithDescriptor:pipelineStateDescriptor error:error];
    if (!_pipelineState) {
        return NO;
    }
    
    _vertexPositionBuffer = [self.device newBufferWithBytes:vertexPositions
                                                     length:sizeof(vertexPositions)
                                                    options:MTLResourceStorageModeShared];
    _texCoordsBuffer = [self.device newBufferWithBytes:texCoords
                                                length:sizeof(texCoords)
                                               options:MTLResourceStorageModeShared];
    
    return YES;
}


- (void)drawTexture:(id<MTLTexture>)texture {
    _texture = texture;
    [self draw];
    _texture = nil;
}


- (void)drawRect:(CGRect)rect {
    MTLRenderPassDescriptor *renderPassDescriptor = self.currentRenderPassDescriptor;
    if (renderPassDescriptor) {
        id<MTLCommandBuffer> commandBuffer = [self.commandQueue commandBuffer];
        id<MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
        
        [renderEncoder setRenderPipelineState:_pipelineState];
        
        [renderEncoder setVertexBuffer:_vertexPositionBuffer offset:0 atIndex:0];
        [renderEncoder setVertexBuffer:_texCoordsBuffer offset:0 atIndex:1];
        
        [renderEncoder setFragmentTexture:_texture atIndex:0];
        
        [renderEncoder drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:numVertices];
        
        [renderEncoder endEncoding];
        [commandBuffer presentDrawable:self.currentDrawable];
        [commandBuffer commit];
    }
}


@end


BEGIN_NAMESPACE_MW


AppleOpenGLContextManager::AppleOpenGLContextManager() :
    contexts(nil),
    views(nil),
    windows(nil)
{
    @autoreleasepool {
        contexts = [[NSMutableArray alloc] init];
        views = [[NSMutableArray alloc] init];
        windows = [[NSMutableArray alloc] init];
    }
}


AppleOpenGLContextManager::~AppleOpenGLContextManager() {
    @autoreleasepool {
        // Set these to nil, so that ARC releases them inside the autorelease pool
        windows = nil;
        views = nil;
        contexts = nil;
    }
}


MWKOpenGLContext * AppleOpenGLContextManager::getContext(int context_id) const  {
    @autoreleasepool {
        if (context_id < 0 || context_id >= contexts.count) {
            merror(M_DISPLAY_MESSAGE_DOMAIN, "OpenGL Context Manager: invalid context ID: %d", context_id);
            return nil;
        }
        return contexts[context_id];
    }
}


MWKOpenGLContext * AppleOpenGLContextManager::getFullscreenContext() const {
    @autoreleasepool {
        if (contexts.count > 0) {
            return contexts[0];
        }
        return nil;
    }
}


MWKOpenGLContext * AppleOpenGLContextManager::getMirrorContext() const  {
    @autoreleasepool {
        if (contexts.count > 1) {
            return contexts[1];
        }
        return getFullscreenContext();
    }
}


MWKMetalView * AppleOpenGLContextManager::getView(int context_id) const  {
    @autoreleasepool {
        if (context_id < 0 || context_id >= views.count) {
            merror(M_DISPLAY_MESSAGE_DOMAIN, "OpenGL Context Manager: invalid context ID: %d", context_id);
            return nil;
        }
        return views[context_id];
    }
}


MWKMetalView * AppleOpenGLContextManager::getFullscreenView() const {
    @autoreleasepool {
        if (views.count > 0) {
            return views[0];
        }
        return nil;
    }
}


MWKMetalView * AppleOpenGLContextManager::getMirrorView() const {
    @autoreleasepool {
        if (views.count > 1) {
            return views[1];
        }
        return getFullscreenView();
    }
}


int AppleOpenGLContextManager::createFramebuffer(int context_id, bool useColorManagement) {
    @autoreleasepool {
        if (auto view = getView(context_id)) {
            return getFramebufferStack(context_id).createFramebuffer();
        }
        return -1;
    }
}


void AppleOpenGLContextManager::pushFramebuffer(int context_id, int framebuffer_id) {
    @autoreleasepool {
        if (auto view = getView(context_id)) {
            getFramebufferStack(context_id).pushFramebuffer(framebuffer_id);
        }
    }
}


void AppleOpenGLContextManager::popFramebuffer(int context_id) {
    @autoreleasepool {
        if (auto view = getView(context_id)) {
            getFramebufferStack(context_id).popFramebuffer();
        }
    }
}


void AppleOpenGLContextManager::flushFramebuffer(int context_id, int framebuffer_id) {
    // Call glFlush so that changes made to the texture are visible to Metal
    glFlush();
}


void AppleOpenGLContextManager::presentFramebuffer(int src_context_id, int framebuffer_id, int dst_context_id) {
    @autoreleasepool {
        if (auto srcView = getView(src_context_id)) {
            if (auto framebufferTexture = getFramebufferStack(src_context_id).getFramebufferTexture(framebuffer_id)) {
                if (auto dstView = getView(dst_context_id)) {
                    [dstView drawTexture:framebufferTexture];
                }
            }
        }
    }
}


void AppleOpenGLContextManager::releaseFramebuffer(int context_id, int framebuffer_id) {
    @autoreleasepool {
        if (auto view = getView(context_id)) {
            getFramebufferStack(context_id).releaseFramebuffer(framebuffer_id);
        }
    }
}


void AppleOpenGLContextManager::releaseFramebuffers() {
    framebufferStacks.clear();
}


AppleOpenGLContextManager::FramebufferStack::FramebufferStack(MWKMetalView *view, MWKOpenGLContext *context) :
    width(view.drawableSize.width),
    height(view.drawableSize.height)
{
    {
        NSDictionary *pixelBufferAttributes = @{
            (NSString *)kCVPixelBufferPixelFormatTypeKey: @(kCVPixelFormatType_64RGBAHalf),
            (NSString *)kCVPixelBufferWidthKey: @(width),
            (NSString *)kCVPixelBufferHeightKey: @(height),
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


int AppleOpenGLContextManager::FramebufferStack::createFramebuffer() {
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
                                                                width,
                                                                height,
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
                                                                   width,
                                                                   height,
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


void AppleOpenGLContextManager::FramebufferStack::pushFramebuffer(int framebuffer_id) {
    auto iter = framebuffers.find(framebuffer_id);
    if (iter == framebuffers.end()) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "OpenGL Context Manager: invalid framebuffer ID: %d", framebuffer_id);
        return;
    }
    stack.emplace_back(iter->second);
    bindCurrentFramebuffer();
}


void AppleOpenGLContextManager::FramebufferStack::popFramebuffer() {
    if (stack.empty()) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "OpenGL Context Manager: no framebuffer to pop");
        return;
    }
    stack.pop_back();
    bindCurrentFramebuffer();
}


void AppleOpenGLContextManager::FramebufferStack::bindCurrentFramebuffer() const {
    if (stack.empty()) {
        // Bind default framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, stack.back().glFramebuffer);
    const GLenum drawBuffer = GL_COLOR_ATTACHMENT0;
    glDrawBuffers(1, &drawBuffer);
    glViewport(0, 0, width, height);
}


id<MTLTexture> AppleOpenGLContextManager::FramebufferStack::getFramebufferTexture(int framebuffer_id) const {
    auto iter = framebuffers.find(framebuffer_id);
    if (iter == framebuffers.end()) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "OpenGL Context Manager: invalid framebuffer ID: %d", framebuffer_id);
        return nil;
    }
    return CVMetalTextureGetTexture(iter->second.cvMetalTexture.get());
}


void AppleOpenGLContextManager::FramebufferStack::releaseFramebuffer(int framebuffer_id) {
    auto iter = framebuffers.find(framebuffer_id);
    if (iter == framebuffers.end()) {
        merror(M_DISPLAY_MESSAGE_DOMAIN, "OpenGL Context Manager: invalid framebuffer ID: %d", framebuffer_id);
        return;
    }
    framebuffers.erase(iter);
}


END_NAMESPACE_MW
