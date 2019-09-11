//
//  MWKCore.h
//  MWorksSwift
//
//  Created by Christopher Stawarz on 9/10/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#import <Foundation/Foundation.h>

#import <MWorksSwift/MWorksSwiftDefines.h>


NS_ASSUME_NONNULL_BEGIN


@class MWKEvent;
typedef void (^MWKEventCallback)(MWKEvent *event);


MWORKSSWIFT_PUBLIC_CLASS
@interface MWKCore : NSObject

- (NSInteger)codeForTag:(NSString *)tag;

- (void)registerCallbackWithKey:(NSString *)key callback:(MWKEventCallback)callback;
- (void)registerCallbackWithKey:(NSString *)key forCode:(NSInteger)code callback:(MWKEventCallback)callback;
- (void)unregisterCallbacksWithKey:(NSString *)key;

@end


NS_ASSUME_NONNULL_END
