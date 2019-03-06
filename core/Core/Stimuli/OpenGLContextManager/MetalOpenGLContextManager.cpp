//
//  MetalOpenGLContextManager.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 3/6/19.
//

#include "MetalOpenGLContextManager.hpp"

#include "OpenGLUtilities.hpp"

#include <Metal/Metal.h>
#include <simd/simd.h>


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
     
     fragment float4
     fragmentShader(RasterizerData in [[stage_in]],
                    texture2d<float> colorTexture [[texture(0)]])
     {
         constexpr sampler textureSampler (mag_filter::linear, min_filter::linear);
         const float4 colorSample = colorTexture.sample(textureSampler, in.textureCoordinate);
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
    id<MTLCommandQueue> _commandQueue;
    id<MTLRenderPipelineState> _pipelineState;
    id<MTLBuffer> _vertexPositionBuffer;
    id<MTLBuffer> _texCoordsBuffer;
    id<MTLTexture> _texture;  // NOT owned
}


- (instancetype)initWithFrame:(CGRect)frameRect device:(id<MTLDevice>)device {
    self = [super initWithFrame:frameRect device:device];
    if (self) {
        self.paused = YES;
        self.enableSetNeedsDisplay = NO;
    }
    return self;
}


- (void)dealloc {
    [_texCoordsBuffer release];
    [_vertexPositionBuffer release];
    [_pipelineState release];
    [_commandQueue release];
    [super dealloc];
}


- (BOOL)prepare {
    _commandQueue = [self.device newCommandQueue];
    
    NSError *error = nullptr;
    id<MTLLibrary> library = [[self.device newLibraryWithSource:@(librarySource.c_str())
                                                        options:nil
                                                          error:&error] autorelease];
    if (!library) {
        mw::merror(mw::M_DISPLAY_MESSAGE_DOMAIN,
                   "Cannot create Metal library: %s",
                   error.localizedDescription.UTF8String);
        return NO;
    }
    
    id<MTLFunction> vertexFunction = [[library newFunctionWithName:@"vertexShader"] autorelease];
    id<MTLFunction> fragmentFunction = [[library newFunctionWithName:@"fragmentShader"] autorelease];
    
    MTLRenderPipelineDescriptor *pipelineStateDescriptor = [[[MTLRenderPipelineDescriptor alloc] init] autorelease];
    pipelineStateDescriptor.vertexFunction = vertexFunction;
    pipelineStateDescriptor.fragmentFunction = fragmentFunction;
    pipelineStateDescriptor.colorAttachments[0].pixelFormat = self.colorPixelFormat;
    
    _pipelineState = [self.device newRenderPipelineStateWithDescriptor:pipelineStateDescriptor error:&error];
    if (!_pipelineState) {
        mw::merror(mw::M_DISPLAY_MESSAGE_DOMAIN,
                   "Cannot create Metal render pipeline state: %s",
                   error.localizedDescription.UTF8String);
        return NO;
    }
    
    _vertexPositionBuffer = [self.device newBufferWithBytes:vertexPositions
                                                     length:sizeof(vertexPositions)
                                                    options:MTLResourceStorageModeShared];
    _texCoordsBuffer = [self.device newBufferWithBytes:texCoords
                                                length:sizeof(texCoords)
                                               options:MTLResourceStorageModeShared];
    
    CGSize drawableSize = self.drawableSize;
    glViewport(0, 0, drawableSize.width, drawableSize.height);
    
    return YES;
}


- (void)drawTexture:(id<MTLTexture>)texture {
    _texture = texture;
    [self draw];
    _texture = nil;
}


- (void)drawRect:(CGRect)rect {
    id<MTLCommandBuffer> commandBuffer = [_commandQueue commandBuffer];
    
    MTLRenderPassDescriptor *renderPassDescriptor = self.currentRenderPassDescriptor;
    if (renderPassDescriptor) {
        id<MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
        
        [renderEncoder setRenderPipelineState:_pipelineState];
        
        [renderEncoder setVertexBuffer:_vertexPositionBuffer offset:0 atIndex:0];
        [renderEncoder setVertexBuffer:_texCoordsBuffer offset:0 atIndex:1];
        
        [renderEncoder setFragmentTexture:_texture atIndex:0];
        
        [renderEncoder drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:numVertices];
        
        [renderEncoder endEncoding];
        [commandBuffer presentDrawable:self.currentDrawable];
    }
    
    [commandBuffer commit];
}


@end


BEGIN_NAMESPACE_MW


MetalOpenGLContextManager::MetalOpenGLContextManager() :
    metalDevice(nil)
{ }


MetalOpenGLContextManager::~MetalOpenGLContextManager() {
    @autoreleasepool {
        [metalDevice release];
    }
}


int MetalOpenGLContextManager::createFramebufferTexture(int context_id, int width, int height, bool srgb) {
    @autoreleasepool {
        GLuint framebufferTexture = 0;
        
        if (auto view = static_cast<MWKMetalView *>(getView(context_id))) {
            {
                NSDictionary *pixelBufferAttributes = @{ (NSString *)kCVPixelBufferMetalCompatibilityKey: @(YES),
#if TARGET_OS_IPHONE
                                                         (NSString *)kCVPixelBufferOpenGLESCompatibilityKey: @(YES),
#else
                                                         (NSString *)kCVPixelBufferOpenGLCompatibilityKey: @(YES),
#endif
                                                         };
                CVPixelBufferRef _cvPixelBuffer = nullptr;
                auto status = CVPixelBufferCreate(kCFAllocatorDefault,
                                                  width,
                                                  height,
                                                  kCVPixelFormatType_32BGRA,
                                                  (CFDictionaryRef)pixelBufferAttributes,
                                                  &_cvPixelBuffer);
                if (status != kCVReturnSuccess) {
                    throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN,
                                          boost::format("Cannot create pixel buffer (error = %d)") % status);
                }
                cvPixelBuffers[context_id] = CVPixelBufferPtr::created(_cvPixelBuffer);
            }
            
            {
                CVMetalTextureCacheRef _cvMetalTextureCache = nullptr;
                auto status = CVMetalTextureCacheCreate(kCFAllocatorDefault,
                                                        nil,
                                                        metalDevice,
                                                        nil,
                                                        &_cvMetalTextureCache);
                if (status != kCVReturnSuccess) {
                    throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN,
                                          boost::format("Cannot create Metal texture cache (error = %d)") % status);
                }
                cvMetalTextureCaches[context_id] = CVMetalTextureCachePtr::created(_cvMetalTextureCache);
            }
            
            {
                CVMetalTextureRef _cvMetalTexture = nullptr;
                auto status = CVMetalTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
                                                                        cvMetalTextureCaches.at(context_id).get(),
                                                                        cvPixelBuffers.at(context_id).get(),
                                                                        nil,
                                                                        MTLPixelFormatBGRA8Unorm,
                                                                        width,
                                                                        height,
                                                                        0,
                                                                        &_cvMetalTexture);
                if (status != kCVReturnSuccess) {
                    throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN,
                                          boost::format("Cannot create Metal texture (error = %d)") % status);
                }
                cvMetalTextures[context_id] = CVMetalTexturePtr::created(_cvMetalTexture);
            }
            
            {
#if TARGET_OS_IPHONE
                CVOpenGLESTextureCacheRef _cvOpenGLTextureCache = nullptr;
                auto status = CVOpenGLESTextureCacheCreate(kCFAllocatorDefault,
                                                           nil,
                                                           getContext(context_id),
                                                           nil,
                                                           &_cvOpenGLTextureCache);
#else
                CVOpenGLTextureCacheRef _cvOpenGLTextureCache = nullptr;
                auto status = CVOpenGLTextureCacheCreate(kCFAllocatorDefault,
                                                         nil,
                                                         getContext(context_id).CGLContextObj,
                                                         getContext(context_id).pixelFormat.CGLPixelFormatObj,
                                                         nil,
                                                         &_cvOpenGLTextureCache);
#endif
                if (status != kCVReturnSuccess) {
                    throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN,
                                          boost::format("Cannot create OpenGL texture cache (error = %d)") % status);
                }
                cvOpenGLTextureCaches[context_id] = CVOpenGLTextureCachePtr::created(_cvOpenGLTextureCache);
            }
            
            {
#if TARGET_OS_IPHONE
                CVOpenGLESTextureRef _cvOpenGLTexture = nullptr;
                auto status = CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
                                                                           cvOpenGLTextureCaches.at(context_id).get(),
                                                                           cvPixelBuffers.at(context_id).get(),
                                                                           nil,
                                                                           GL_TEXTURE_2D,
                                                                           (srgb ? GL_SRGB8_ALPHA8 : GL_RGBA8),
                                                                           width,
                                                                           height,
                                                                           GL_BGRA_EXT,
                                                                           GL_UNSIGNED_BYTE,
                                                                           0,
                                                                           &_cvOpenGLTexture);
#else
                CVOpenGLTextureRef _cvOpenGLTexture = nullptr;
                auto status = CVOpenGLTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
                                                                         cvOpenGLTextureCaches.at(context_id).get(),
                                                                         cvPixelBuffers.at(context_id).get(),
                                                                         nil,
                                                                         &_cvOpenGLTexture);
#endif
                if (status != kCVReturnSuccess) {
                    throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN,
                                          boost::format("Cannot create OpenGL texture (error = %d)") % status);
                }
                cvOpenGLTextures[context_id] = CVOpenGLTexturePtr::created(_cvOpenGLTexture);
            }
            
#if TARGET_OS_IPHONE
            framebufferTexture = CVOpenGLESTextureGetName(cvOpenGLTextures.at(context_id).get());
#else
            framebufferTexture = CVOpenGLTextureGetName(cvOpenGLTextures.at(context_id).get());
#endif
        }
        
        return framebufferTexture;
    }
}


void MetalOpenGLContextManager::flushFramebufferTexture(int context_id) {
    // Call glFlush so that changes made to the texture are visible to Metal
    glFlush();
}


void MetalOpenGLContextManager::drawFramebufferTexture(int src_context_id, int dst_context_id) {
    @autoreleasepool {
        if (auto view = static_cast<MWKMetalView *>(getView(dst_context_id))) {
            [view drawTexture:CVMetalTextureGetTexture(cvMetalTextures.at(src_context_id).get())];
        }
    }
}


void MetalOpenGLContextManager::releaseFramebufferTextures() {
    cvOpenGLTextures.clear();
    cvOpenGLTextureCaches.clear();
    cvMetalTextures.clear();
    cvMetalTextureCaches.clear();
    cvPixelBuffers.clear();
}


END_NAMESPACE_MW
