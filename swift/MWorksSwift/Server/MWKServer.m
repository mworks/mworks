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


- (BOOL)openExperimentAtPath:(NSString *)path {
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


- (void)stopExperiment {
    try {
        self.server->stopExperiment();
    } catch (...) {
        MWorksSwiftLogException(std::current_exception());
    }
}


@end
