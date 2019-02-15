//
//  IOSOpenGLContextManager.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 2/14/17.
//
//

#include "IOSOpenGLContextManager.hpp"

#include "OpenGLUtilities.hpp"

#include <Metal/Metal.h>
#include <MetalKit/MetalKit.h>
#include <OpenGLES/ES3/glext.h>
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


@interface MWKMetalView : MTKView

@property(nonatomic, readonly) EAGLContext *context;

- (instancetype)initWithFrame:(CGRect)frameRect device:(id<MTLDevice>)device context:(EAGLContext *)context;
- (mw::OpenGLContextLock)lockContext;
- (BOOL)prepare;
- (void)drawTexture:(id<MTLTexture>)texture;

@end


@implementation MWKMetalView {
    mw::OpenGLContextLock::unique_lock::mutex_type mutex;
    id<MTLCommandQueue> _commandQueue;
    id<MTLRenderPipelineState> _pipelineState;
    id<MTLBuffer> _vertexPositionBuffer;
    id<MTLBuffer> _texCoordsBuffer;
    id<MTLTexture> _texture;  // NOT owned
}


- (instancetype)initWithFrame:(CGRect)frameRect device:(id<MTLDevice>)device context:(EAGLContext *)context {
    self = [super initWithFrame:frameRect device:device];
    if (self) {
        _context = [context retain];
    }
    return self;
}


- (void)dealloc {
    [_texCoordsBuffer release];
    [_vertexPositionBuffer release];
    [_pipelineState release];
    [_commandQueue release];
    [_context release];
    [super dealloc];
}


- (mw::OpenGLContextLock)lockContext {
    return mw::OpenGLContextLock(mw::OpenGLContextLock::unique_lock(mutex));
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


@interface MWKMetalViewController : UIViewController
@end


@implementation MWKMetalViewController


- (BOOL)prefersStatusBarHidden {
    return YES;
}


- (BOOL)shouldAutorotate {
    return NO;
}


@end


BEGIN_NAMESPACE_MW


IOSOpenGLContextManager::IOSOpenGLContextManager() :
    metalDevice(nil)
{
    @autoreleasepool {
        metalDevice = MTLCreateSystemDefaultDevice();
        if (!metalDevice) {
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Metal is not supported on this device");
        }
    }
}


IOSOpenGLContextManager::~IOSOpenGLContextManager() {
    @autoreleasepool {
        // Calling releaseContexts here causes the application to crash at exit.  Since this class is
        // used as a singleton, it doesn't matter, anyway.
        //releaseContexts();
        [metalDevice release];
    }
}


int IOSOpenGLContextManager::newFullscreenContext(int screen_number, bool render_at_full_resolution, bool opaque) {
    //
    // NOTE: We always make iOS windows opaque, as there's nothing to show through them from behind
    //
    
    @autoreleasepool {
        if (screen_number < 0 || screen_number >= getNumDisplays()) {
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN,
                                  (boost::format("Invalid screen number (%d)") % screen_number).str());
        }
        
        EAGLSharegroup *sharegroup = nil;
        if (contexts.count > 0) {
            sharegroup = static_cast<EAGLContext *>(contexts[0]).sharegroup;
        }
        
        EAGLContext *context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3 sharegroup:sharegroup];
        if (!context) {
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Cannot create OpenGL ES context");
        }
        
        auto screen = UIScreen.screens[screen_number];
        __block bool success = false;
        
        dispatch_sync(dispatch_get_main_queue(), ^{
            if (UIWindow *window = [[UIWindow alloc] initWithFrame:screen.bounds]) {
                window.screen = screen;
                
                if (MWKMetalViewController *viewController = [[MWKMetalViewController alloc] init]) {
                    window.rootViewController = viewController;
                    
                    if (MWKMetalView *view = [[MWKMetalView alloc] initWithFrame:window.bounds
                                                                          device:metalDevice
                                                                         context:context])
                    {
                        viewController.view = view;
                        view.contentScaleFactor = (render_at_full_resolution ? screen.nativeScale : 1.0);
                        view.paused = YES;
                        view.enableSetNeedsDisplay = NO;
                        [EAGLContext setCurrentContext:context];
                        
                        if ([view prepare]) {
                            [window makeKeyAndVisible];
                            
                            [contexts addObject:context];
                            [views addObject:view];
                            [windows addObject:window];
                            
                            success = true;
                        }
                        
                        [EAGLContext setCurrentContext:nil];
                        [view release];
                    }
                    
                    [viewController release];
                }
                
                [window release];
            }
        });
        
        [context release];
        
        if (!success) {
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Cannot create fullscreen window");
        }
        
        return (contexts.count - 1);
    }
}


int IOSOpenGLContextManager::newMirrorContext(bool render_at_full_resolution) {
    throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN, "Mirror windows are not supported on this OS");
}


void IOSOpenGLContextManager::releaseContexts() {
    @autoreleasepool {
        cvOpenGLESTextures.clear();
        cvOpenGLESTextureCaches.clear();
        cvMetalTextures.clear();
        cvMetalTextureCaches.clear();
        cvPixelBuffers.clear();
        
        dispatch_sync(dispatch_get_main_queue(), ^{
            for (UIWindow *window in windows) {
                window.hidden = YES;
            }
            [windows removeAllObjects];
            [views removeAllObjects];
        });
        
        [contexts removeAllObjects];
    }
}


int IOSOpenGLContextManager::getNumDisplays() const {
    // At present, we support only the main display
    return 1;
}


OpenGLContextLock IOSOpenGLContextManager::setCurrent(int context_id) {
    @autoreleasepool {
        if (auto view = static_cast<MWKMetalView *>(getView(context_id))) {
            if ([EAGLContext setCurrentContext:view.context]) {
                return [view lockContext];
            }
            merror(M_DISPLAY_MESSAGE_DOMAIN, "Cannot set current OpenGL ES context");
        }
        return OpenGLContextLock();
    }
}


void IOSOpenGLContextManager::clearCurrent() {
    @autoreleasepool {
        [EAGLContext setCurrentContext:nil];
    }
}


void IOSOpenGLContextManager::prepareContext(int context_id, bool useColorManagement) {
    if (useColorManagement) {
        checkDisplayGamut(context_id);
    }
}


int IOSOpenGLContextManager::createFramebufferTexture(int context_id, int width, int height, bool srgb) {
    @autoreleasepool {
        GLuint framebufferTexture = 0;
        
        if (auto view = static_cast<MWKMetalView *>(getView(context_id))) {
            {
                NSDictionary *pixelBufferAttributes = @{ (NSString *)kCVPixelBufferMetalCompatibilityKey: @(YES),
                                                         (NSString *)kCVPixelBufferOpenGLESCompatibilityKey: @(YES) };
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
                CVOpenGLESTextureCacheRef _cvOpenGLESTextureCache = nullptr;
                auto status = CVOpenGLESTextureCacheCreate(kCFAllocatorDefault,
                                                           nil,
                                                           view.context,
                                                           nil,
                                                           &_cvOpenGLESTextureCache);
                if (status != kCVReturnSuccess) {
                    throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN,
                                          boost::format("Cannot create OpenGL ES texture cache (error = %d)") % status);
                }
                cvOpenGLESTextureCaches[context_id] = CVOpenGLESTextureCachePtr::created(_cvOpenGLESTextureCache);
            }
            
            {
                CVOpenGLESTextureRef _cvOpenGLESTexture = nullptr;
                auto status = CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
                                                                           cvOpenGLESTextureCaches.at(context_id).get(),
                                                                           cvPixelBuffers.at(context_id).get(),
                                                                           nil,
                                                                           GL_TEXTURE_2D,
                                                                           (srgb ? GL_SRGB8_ALPHA8 : GL_RGBA8),
                                                                           width,
                                                                           height,
                                                                           GL_BGRA_EXT,
                                                                           GL_UNSIGNED_BYTE,
                                                                           0,
                                                                           &_cvOpenGLESTexture);
                if (status != kCVReturnSuccess) {
                    throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN,
                                          boost::format("Cannot create OpenGL ES texture (error = %d)") % status);
                }
                cvOpenGLESTextures[context_id] = CVOpenGLESTexturePtr::created(_cvOpenGLESTexture);
            }
            
            framebufferTexture = CVOpenGLESTextureGetName(cvOpenGLESTextures.at(context_id).get());
        }
        
        return framebufferTexture;
    }
}


void IOSOpenGLContextManager::flushFramebufferTexture(int context_id) {
    // Call glFlush so that changes made to the texture are visible to Metal
    glFlush();
}


void IOSOpenGLContextManager::drawFramebufferTexture(int src_context_id, int dst_context_id) {
    @autoreleasepool {
        if (auto view = static_cast<MWKMetalView *>(getView(dst_context_id))) {
            [view drawTexture:CVMetalTextureGetTexture(cvMetalTextures.at(src_context_id).get())];
        }
    }
}


void IOSOpenGLContextManager::checkDisplayGamut(int context_id) const {
    @autoreleasepool {
        if (auto view = getView(context_id)) {
            __block UIDisplayGamut displayGamut;
            dispatch_sync(dispatch_get_main_queue(), ^{
                displayGamut = view.window.screen.traitCollection.displayGamut;
            });
            
            switch (displayGamut) {
                case UIDisplayGamutSRGB:
                    // No color conversion required
                    break;
                    
                case UIDisplayGamutP3: {
                    //
                    // According to "What's New in Metal, Part 2" (WWDC 2016, Session 605), applications should
                    // always render in the sRGB colorspace, even when the target device has a P3 display.  To use
                    // colors outside of the sRGB gamut, the app needs to use a floating-point color buffer and
                    // encode the P3-only colors using component values less than 0 or greater than 1 (as in Apple's
                    // "extended sRGB" color space).  Since StimulusDisplay uses an 8 bit per channel, integer color
                    // buffer, we're always limited to sRGB.
                    //
                    // Testing suggests that this approach is correct.  If we draw an image with high color
                    // saturation and then display it both with and without a Mac-style, LUT-based conversion
                    // from sRGB to Display P3, the unconverted colors match what we see on a Mac, while the converted
                    // colors are noticeably duller.  This makes sense, because converting, say, 100% red (255, 0, 0)
                    // in sRGB to the wider gamut of Display P3 results in smaller numerical values (234, 51, 35).
                    //
                    // https://developer.apple.com/videos/play/wwdc2016/605/
                    //
                    break;
                }
                    
                default:
                    merror(M_DISPLAY_MESSAGE_DOMAIN, "Unsupported display gamut");
                    break;
            }
        }
    }
}


boost::shared_ptr<OpenGLContextManager> OpenGLContextManager::createPlatformOpenGLContextManager() {
    return boost::make_shared<IOSOpenGLContextManager>();
}


END_NAMESPACE_MW


























