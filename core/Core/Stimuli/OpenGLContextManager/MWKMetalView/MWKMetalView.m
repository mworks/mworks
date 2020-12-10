//
//  MWKMetalView.m
//  MWorksCore
//
//  Created by Christopher Stawarz on 12/11/20.
//

#import "MWKMetalView_Private.h"


@implementation MWKMetalView {
    id<MTLRenderPipelineState> _pipelineState;
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
    id<MTLLibrary> library = [self.device newDefaultLibraryWithBundle:[NSBundle bundleForClass:[self class]]
                                                                error:error];
    if (!library) {
        return NO;
    }
    
    MTLFunctionConstantValues *functionConstantValues = [[MTLFunctionConstantValues alloc] init];
    const bool convertToSRGB = useColorManagement;
    [functionConstantValues setConstantValue:&convertToSRGB
                                        type:MTLDataTypeBool
                                    withName:@"MWKMetalView_convertToSRGB"];
    
    id<MTLFunction> vertexFunction = [library newFunctionWithName:@"MWKMetalView_vertexShader"];
    
    id<MTLFunction> fragmentFunction = [library newFunctionWithName:@"MWKMetalView_fragmentShader"
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
        [renderEncoder setFragmentTexture:_texture atIndex:0];
        
        [renderEncoder drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];
        
        [renderEncoder endEncoding];
        [commandBuffer presentDrawable:self.currentDrawable];
        [commandBuffer commit];
    }
}


@end
