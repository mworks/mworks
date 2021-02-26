//
//  MWKTripleBufferedMTLResource.h
//  MWorksCore
//
//  Created by Christopher Stawarz on 2/26/21.
//

#ifndef MWKTripleBufferedMTLResource_h
#define MWKTripleBufferedMTLResource_h

#import <Metal/Metal.h>


NS_ASSUME_NONNULL_BEGIN


//
// This class supports synchronization of Metal resource usage between the CPU and GPU, as described at
// https://developer.apple.com/documentation/metal/synchronization/synchronizing_cpu_and_gpu_work
//

__attribute__((visibility("default")))
@interface MWKTripleBufferedMTLResource<__covariant ResourceType> : NSObject

+ (MWKTripleBufferedMTLResource<id<MTLBuffer>> *)bufferWithDevice:(id<MTLDevice>)device
                                                           length:(NSUInteger)length
                                                          options:(MTLResourceOptions)options;

+ (MWKTripleBufferedMTLResource<id<MTLTexture>> *)textureWithDevice:(id<MTLDevice>)device
                                                         descriptor:(MTLTextureDescriptor *)descriptor;

- (ResourceType)acquireWithCommandBuffer:(id<MTLCommandBuffer>)commandBuffer;

@end


NS_ASSUME_NONNULL_END


#endif /* MWKTripleBufferedMTLResource_h */
