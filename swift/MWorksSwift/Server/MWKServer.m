//
//  MWKServer.m
//  MWorksSwift
//
//  Created by Christopher Stawarz on 9/5/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#import "MWKServer_Private.h"

#import "MWorksSwiftErrors_Private.h"
#import "MWKCore_Private.h"


@implementation MWKServer {
    boost::shared_ptr<mw::Server> _server;
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
    MWKServer *result = nil;
    try {
        auto server = boost::make_shared<mw::Server>();
        server->setHostname(address.UTF8String);
        server->setListenPort(port);
        mw::Server::registerInstance(server);
        result = [[MWKServer alloc] initWithServer:server];
    } catch (...) {
        if (error) {
            *error = MWorksSwiftConvertExceptionToNSError(std::current_exception());
        }
    }
    return result;
}


- (instancetype)initWithServer:(const boost::shared_ptr<mw::Server> &)server {
    self = [super initWithCore:server];
    if (self) {
        _server = server;
    }
    return self;
}


- (const boost::shared_ptr<mw::Server> &)server {
    return _server;
}


- (BOOL)start {
    BOOL result = NO;
    try {
        result = self.server->startServer();
    } catch (...) {
        MWorksSwiftLogException(std::current_exception());
    }
    return result;
}


- (BOOL)stop {
    try {
        self.server->stopServer();
    } catch (...) {
        MWorksSwiftLogException(std::current_exception());
    }
    return YES;
}


- (BOOL)openExperiment:(NSString *)path {
    BOOL result = NO;
    try {
        result = self.server->openExperiment(path.UTF8String);
    } catch (...) {
        MWorksSwiftLogException(std::current_exception());
    }
    return result;
}


- (BOOL)closeExperiment {
    BOOL result = NO;
    try {
        result = self.server->closeExperiment();
    } catch (...) {
        MWorksSwiftLogException(std::current_exception());
    }
    return result;
}


- (void)startExperiment {
    try {
        self.server->startExperiment();
    } catch (...) {
        MWorksSwiftLogException(std::current_exception());
    }
}


@end
