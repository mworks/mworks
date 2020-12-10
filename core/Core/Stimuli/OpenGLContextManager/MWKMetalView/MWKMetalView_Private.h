//
//  MWKMetalView_Private.h
//  MWorksCore
//
//  Created by Christopher Stawarz on 12/11/20.
//

#ifndef MWKMetalView_Private_h
#define MWKMetalView_Private_h

#import "MWKMetalView.h"


NS_ASSUME_NONNULL_BEGIN


@interface MWKMetalView ()

- (BOOL)prepareUsingColorManagement:(BOOL)useColorManagement error:(__autoreleasing NSError **)error;
- (void)drawTexture:(id<MTLTexture>)texture;

@end


NS_ASSUME_NONNULL_END


#endif /* MWKMetalView_Private_h */
