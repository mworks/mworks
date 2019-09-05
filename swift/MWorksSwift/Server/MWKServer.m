//
//  MWKServer.m
//  MWorksSwift
//
//  Created by Christopher Stawarz on 9/5/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#import "MWKServer.h"

#import "MWorksSwiftErrors_Private.h"
#import "MWKEvent_Private.h"


@implementation MWKServer {
    boost::shared_ptr<mw::Server> core;
}


+ (BOOL)constructCore:(NSError **)error {
    BOOL result = NO;
    try {
        static std::once_flag coreConstructedFlag;
        std::call_once(coreConstructedFlag, []() {
            mw::StandardServerCoreBuilder coreBuilder;
            mw::CoreBuilderForeman::constructCoreStandardOrder(&coreBuilder);
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


+ (NSString *)hostName {
    NSString *result = @"";
    try {
        std::string hostname;
        if (mw::zeromq::getHostname(hostname)) {
            result = @(hostname.c_str());
        }
    } catch (...) {
        MWorksSwiftLogException(std::current_exception());
    }
    return result;
}


+ (instancetype)serverWithListeningAddress:(NSString *)address
                             listeningPort:(NSInteger)port
                                     error:(NSError **)error
{
    MWKServer *server = nil;
    try {
        auto core = boost::make_shared<mw::Server>();
        core->setHostname(address.UTF8String);
        core->setListenPort(port);
        mw::Server::registerInstance(core);
        server = [[MWKServer alloc] initWithCore:core];
    } catch (...) {
        if (error) {
            *error = MWorksSwiftConvertExceptionToNSError(std::current_exception());
        }
    }
    return server;
}


- (instancetype)initWithCore:(const boost::shared_ptr<mw::Server> &)core {
    self = [super init];
    if (self) {
        self->core = core;
    }
    return self;
}


- (BOOL)start {
    BOOL result = NO;
    try {
        result = core->startServer();
    } catch (...) {
        MWorksSwiftLogException(std::current_exception());
    }
    return result;
}


- (BOOL)stop {
    try {
        core->stopServer();
    } catch (...) {
        MWorksSwiftLogException(std::current_exception());
    }
    return YES;
}


- (NSInteger)codeForTag:(NSString *)tag {
    NSInteger code = -1;
    try {
        code = core->lookupCodeForTag(tag.UTF8String);
    } catch (...) {
        MWorksSwiftLogException(std::current_exception());
    }
    return code;
}


static inline mw::EventCallback wrapCallback(MWKEventCallback callback) {
    return [callback](const boost::shared_ptr<mw::Event> &event) {
        callback([MWKEvent eventWithEvent:event]);
    };
}


- (void)registerCallbackWithKey:(NSString *)key callback:(MWKEventCallback)callback {
    try {
        core->registerCallback(wrapCallback(callback), key.UTF8String);
    } catch (...) {
        MWorksSwiftLogException(std::current_exception());
    }
}


- (void)registerCallbackWithKey:(NSString *)key forCode:(NSInteger)code callback:(MWKEventCallback)callback {
    try {
        core->registerCallback(code, wrapCallback(callback), key.UTF8String);
    } catch (...) {
        MWorksSwiftLogException(std::current_exception());
    }
}


- (void)unregisterCallbacksWithKey:(NSString *)key {
    try {
        core->unregisterCallbacks(key.UTF8String);
    } catch (...) {
        MWorksSwiftLogException(std::current_exception());
    }
}


- (BOOL)openExperiment:(NSString *)path {
    BOOL result = NO;
    try {
        result = core->openExperiment(path.UTF8String);
    } catch (...) {
        MWorksSwiftLogException(std::current_exception());
    }
    return result;
}


- (BOOL)closeExperiment {
    BOOL result = NO;
    try {
        result = core->closeExperiment();
    } catch (...) {
        MWorksSwiftLogException(std::current_exception());
    }
    return result;
}


- (void)startExperiment {
    try {
        core->startExperiment();
    } catch (...) {
        MWorksSwiftLogException(std::current_exception());
    }
}


@end
