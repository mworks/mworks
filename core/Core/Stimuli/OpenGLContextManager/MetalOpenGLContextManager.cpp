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


- (BOOL)prepareUsingColorManagement:(BOOL)useColorManagement error:(NSError **)error {
    _commandQueue = [self.device newCommandQueue];
    
    id<MTLLibrary> library = [[self.device newLibraryWithSource:@(librarySource.c_str())
                                                        options:nil
                                                          error:error] autorelease];
    if (!library) {
        return NO;
    }
    
    MTLFunctionConstantValues *functionConstantValues = [[[MTLFunctionConstantValues alloc] init] autorelease];
    const bool convertToSRGB = useColorManagement;
    [functionConstantValues setConstantValue:&convertToSRGB type:MTLDataTypeBool atIndex:0];
    
    id<MTLFunction> vertexFunction = [[library newFunctionWithName:@"vertexShader"] autorelease];
    
    id<MTLFunction> fragmentFunction = [[library newFunctionWithName:@"fragmentShader"
                                                      constantValues:functionConstantValues
                                                               error:error] autorelease];
    if (!fragmentFunction) {
        return NO;
    }
    
    MTLRenderPipelineDescriptor *pipelineStateDescriptor = [[[MTLRenderPipelineDescriptor alloc] init] autorelease];
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


int MetalOpenGLContextManager::createFramebufferTexture(int context_id,
                                                        bool useColorManagement,
                                                        int &target,
                                                        int &width,
                                                        int &height)
{
    @autoreleasepool {
        GLuint framebufferTexture = 0;
        
        if (auto view = static_cast<MWKMetalView *>(getView(context_id))) {
            {
                CGSize drawableSize = view.drawableSize;
                width = drawableSize.width;
                height = drawableSize.height;
            }
            
            {
                NSDictionary *pixelBufferAttributes = @{ (NSString *)kCVPixelBufferMetalCompatibilityKey: @(YES),
#if TARGET_OS_OSX
                                                         (NSString *)kCVPixelBufferOpenGLCompatibilityKey: @(YES),
#else
                                                         (NSString *)kCVPixelBufferOpenGLESCompatibilityKey: @(YES),
#endif
                                                         };
                CVPixelBufferRef _cvPixelBuffer = nullptr;
                auto status = CVPixelBufferCreate(kCFAllocatorDefault,
                                                  width,
                                                  height,
                                                  kCVPixelFormatType_64RGBAHalf,
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
                                                        view.device,
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
                                                                        MTLPixelFormatRGBA16Float,
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
#if TARGET_OS_OSX
                NSOpenGLContext *context = getContext(context_id);
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
                                                           getContext(context_id),
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
#if TARGET_OS_OSX
                CVOpenGLTextureRef _cvOpenGLTexture = nullptr;
                auto status = CVOpenGLTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
                                                                         cvOpenGLTextureCaches.at(context_id).get(),
                                                                         cvPixelBuffers.at(context_id).get(),
                                                                         nil,
                                                                         &_cvOpenGLTexture);
#else
                CVOpenGLESTextureRef _cvOpenGLTexture = nullptr;
                auto status = CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
                                                                           cvOpenGLTextureCaches.at(context_id).get(),
                                                                           cvPixelBuffers.at(context_id).get(),
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
                cvOpenGLTextures[context_id] = CVOpenGLTexturePtr::created(_cvOpenGLTexture);
                
#if TARGET_OS_OSX
                target = CVOpenGLTextureGetTarget(_cvOpenGLTexture);
                framebufferTexture = CVOpenGLTextureGetName(_cvOpenGLTexture);
#else
                target = CVOpenGLESTextureGetTarget(_cvOpenGLTexture);
                framebufferTexture = CVOpenGLESTextureGetName(_cvOpenGLTexture);
#endif
            }
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
