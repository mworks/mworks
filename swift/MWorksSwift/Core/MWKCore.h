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


typedef NS_ENUM(NSInteger, MWKCoreType) {
    MWKCoreTypeClient,
    MWKCoreTypeServer
} NS_SWIFT_NAME(Core.Type);


@class MWKEvent;
typedef void (^MWKEventCallback)(MWKEvent *event);


NS_SWIFT_NAME(CoreProtocol)
@protocol MWKCore <NSObject>

- (NSInteger)codeForTag:(NSString *)tag;

- (void)registerCallbackWithKey:(NSString *)key callback:(MWKEventCallback)callback;
- (void)registerCallbackWithKey:(NSString *)key forCode:(NSInteger)code callback:(MWKEventCallback)callback;
- (void)registerCallbackWithKey:(NSString *)key forTag:(NSString *)tag callback:(MWKEventCallback)callback;

- (void)unregisterCallbacksWithKey:(NSString *)key;

@end


MWORKSSWIFT_PUBLIC_CLASS
NS_SWIFT_NAME(Core)
@interface MWKCore : NSObject <MWKCore>

+ (BOOL)constructCoreWithType:(MWKCoreType)type error:(NSError * _Nullable *)error;

@end


NS_ASSUME_NONNULL_END
