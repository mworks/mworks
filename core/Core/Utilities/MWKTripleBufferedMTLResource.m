//
//  MWKTripleBufferedMTLResource.m
//  MWorksCore
//
//  Created by Christopher Stawarz on 2/26/21.
//

#import "MWKTripleBufferedMTLResource.h"

#define RESOURCE_POOL_SIZE 3


@implementation MWKTripleBufferedMTLResource {
    dispatch_semaphore_t _resourcePoolSemaphore;
    NSUInteger _currentResourceIndex;
    NSArray<id<MTLResource>> *_resourcePool;
}


+ (MWKTripleBufferedMTLResource<id<MTLBuffer>> *)bufferWithDevice:(id<MTLDevice>)device
                                                           length:(NSUInteger)length
                                                          options:(MTLResourceOptions)options
{
    return [[self alloc] initWithResourceFactory:^{
        return [device newBufferWithLength:length options:options];
    }];
}


+ (MWKTripleBufferedMTLResource<id<MTLTexture>> *)textureWithDevice:(id<MTLDevice>)device
                                                         descriptor:(MTLTextureDescriptor *)descriptor
{
    return [[self alloc] initWithResourceFactory:^{
        return [device newTextureWithDescriptor:descriptor];
    }];
}


- (instancetype)initWithResourceFactory:(id<MTLResource> (^)(void))resourceFactory {
    self = [super init];
    if (self) {
        _resourcePoolSemaphore = dispatch_semaphore_create(RESOURCE_POOL_SIZE);
        _currentResourceIndex = RESOURCE_POOL_SIZE - 1;
        
        NSMutableArray<id<MTLResource>> *resourcePool = [NSMutableArray arrayWithCapacity:RESOURCE_POOL_SIZE];
        do {
            [resourcePool addObject:resourceFactory()];
        } while (resourcePool.count < RESOURCE_POOL_SIZE);
        _resourcePool = [resourcePool copy];
    }
    return self;
}


- (id<MTLResource>)acquireWithCommandBuffer:(id<MTLCommandBuffer>)commandBuffer {
    // Wait until a resource is available
    dispatch_semaphore_wait(_resourcePoolSemaphore, DISPATCH_TIME_FOREVER);
    
    // Register a completed handler, so we know when the GPU is done with the resource
    {
        __block dispatch_semaphore_t semaphore = _resourcePoolSemaphore;
        [commandBuffer addCompletedHandler:^(id<MTLCommandBuffer> commandBuffer) {
            dispatch_semaphore_signal(semaphore);
        }];
    }
    
    // Return the resource
    _currentResourceIndex = (_currentResourceIndex + 1) % RESOURCE_POOL_SIZE;
    return _resourcePool[_currentResourceIndex];
}


@end
