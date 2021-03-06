//
//  MWKCore.m
//  MWorksSwift
//
//  Created by Christopher Stawarz on 9/10/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#import "MWKCore_Private.h"

#import "MWorksSwiftErrors_Private.h"
#import "MWKEvent_Private.h"


@implementation MWKCore {
    boost::shared_ptr<mw::RegistryAwareEventStreamInterface> _core;
}


+ (BOOL)constructCoreWithType:(MWKCoreType)type error:(NSError **)error {
    BOOL result = NO;
    try {
        static std::once_flag coreConstructedFlag;
        std::call_once(coreConstructedFlag, [type]() {
            std::unique_ptr<mw::AbstractCoreBuilder> coreBuilder;
            if (MWKCoreTypeClient == type) {
                coreBuilder.reset(new mw::StandardClientCoreBuilder);
            } else {
                coreBuilder.reset(new mw::StandardServerCoreBuilder);
            }
            mw::CoreBuilderForeman::constructCoreStandardOrder(coreBuilder.get());
        });
        result = YES;
    } catch (mw::ComponentFactoryConflictException &e) {
        if (error) {
            *error = [NSError errorWithDomain:MWorksSwiftErrorDomain
                                         code:MWorksSwiftErrorComponentFactoryConflict
                                     userInfo:@{
                                                NSLocalizedDescriptionKey: @(e.getMessage().c_str()),
                                                NSLocalizedRecoverySuggestionErrorKey:
                                                    @"You must review your plugins to ensure that multiple plugins"
                                                    " aren't trying to register functionality under the same XML"
                                                    " signatures"
                                                }];
        }
    } catch (...) {
        if (error) {
            *error = MWorksSwiftConvertExceptionToNSError(std::current_exception());
        }
    }
    return result;
}


- (instancetype)initWithCore:(const boost::shared_ptr<mw::RegistryAwareEventStreamInterface> &)core {
    self = [super init];
    if (self) {
        _core = core;
    }
    return self;
}


- (const boost::shared_ptr<mw::RegistryAwareEventStreamInterface> &)core {
    return _core;
}


- (NSInteger)codeForTag:(NSString *)tag {
    NSInteger code = -1;
    try {
        code = self.core->lookupCodeForTag(tag.UTF8String);
    } catch (...) {
        MWorksSwiftLogException(std::current_exception());
    }
    return code;
}


static inline mw::EventCallback wrapCallback(MWKEventCallback callback) {
    return [callback](const boost::shared_ptr<mw::Event> &event) {
        callback([[MWKEvent alloc] initWithEvent:event]);
    };
}


- (void)registerCallbackWithKey:(NSString *)key callback:(MWKEventCallback)callback {
    try {
        self.core->registerCallback(wrapCallback(callback), key.UTF8String);
    } catch (...) {
        MWorksSwiftLogException(std::current_exception());
    }
}


- (void)registerCallbackWithKey:(NSString *)key forCode:(NSInteger)code callback:(MWKEventCallback)callback {
    try {
        self.core->registerCallback(code, wrapCallback(callback), key.UTF8String);
    } catch (...) {
        MWorksSwiftLogException(std::current_exception());
    }
}


- (void)registerCallbackWithKey:(NSString *)key forTag:(NSString *)tag callback:(MWKEventCallback)callback {
    try {
        self.core->registerCallback(tag.UTF8String, wrapCallback(callback), key.UTF8String);
    } catch (...) {
        MWorksSwiftLogException(std::current_exception());
    }
}


- (void)unregisterCallbacksWithKey:(NSString *)key {
    try {
        self.core->unregisterCallbacks(key.UTF8String);
    } catch (...) {
        MWorksSwiftLogException(std::current_exception());
    }
}


@end
