//
//  MWKMetalView.h
//  MWorksCore
//
//  Created by Christopher Stawarz on 12/11/20.
//

#ifndef MWKMetalView_h
#define MWKMetalView_h

#import <MetalKit/MetalKit.h>


NS_ASSUME_NONNULL_BEGIN


@interface MWKMetalView : MTKView

@property(nonatomic, readonly) id<MTLCommandQueue> commandQueue;

@end


NS_ASSUME_NONNULL_END


#endif /* MWKMetalView_h */
