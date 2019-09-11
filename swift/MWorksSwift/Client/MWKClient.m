//
//  MWKClient.m
//  MWorksSwift
//
//  Created by Christopher Stawarz on 9/11/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#import "MWKClient_Private.h"

#import "MWorksSwiftErrors_Private.h"
#import "MWKCore_Private.h"


@implementation MWKClient {
    boost::shared_ptr<mw::Client> _client;
}


+ (BOOL)constructCore:(NSError **)error {
    BOOL result = NO;
    try {
        static std::once_flag coreConstructedFlag;
        std::call_once(coreConstructedFlag, []() {
            mw::StandardClientCoreBuilder coreBuilder;
            mw::CoreBuilderForeman::constructCoreStandardOrder(&coreBuilder);
        });
        result = YES;
    } catch (...) {
        if (error) {
            *error = MWorksSwiftConvertExceptionToNSError(std::current_exception());
        }
    }
    return result;
}


+ (instancetype)client:(NSError **)error {
    MWKClient *result = nil;
    try {
        auto client = boost::make_shared<mw::Client>();
        result = [[MWKClient alloc] initWithClient:client];
    } catch (...) {
        if (error) {
            *error = MWorksSwiftConvertExceptionToNSError(std::current_exception());
        }
    }
    return result;
}


- (instancetype)initWithClient:(const boost::shared_ptr<mw::Client> &)client {
    self = [super initWithCore:client];
    if (self) {
        _client = client;
    }
    return self;
}


- (const boost::shared_ptr<mw::Client> &)client {
    return _client;
}


- (void)startEventListener {
    try {
        self.client->startEventListener();
    } catch (...) {
        MWorksSwiftLogException(std::current_exception());
    }
}


- (BOOL)connected {
    BOOL result = NO;
    try {
        result = self.client->isConnected();
    } catch (...) {
        MWorksSwiftLogException(std::current_exception());
    }
    return result;
}


- (BOOL)connectToServer:(NSString *)address port:(NSInteger)port {
    BOOL result = NO;
    try {
        result = self.client->connectToServer(address.UTF8String, port);
    } catch (...) {
        MWorksSwiftLogException(std::current_exception());
    }
    return result;
}


- (BOOL)disconnect {
    BOOL result = NO;
    try {
        result = self.client->disconnectClient();
    } catch (...) {
        MWorksSwiftLogException(std::current_exception());
    }
    return result;
}


- (BOOL)sendExperiment:(NSString *)path {
    BOOL result = NO;
    try {
        result = self.client->sendExperiment(path.UTF8String);
    } catch (...) {
        MWorksSwiftLogException(std::current_exception());
    }
    return result;
}


- (void)sendCloseExperimentEvent:(NSString *)path {
    try {
        self.client->sendCloseExperimentEvent(path.UTF8String);
    } catch (...) {
        MWorksSwiftLogException(std::current_exception());
    }
}


- (void)sendProtocolSelectedEvent:(NSString *)protocolName {
    try {
        self.client->sendProtocolSelectedEvent(protocolName.UTF8String);
    } catch (...) {
        MWorksSwiftLogException(std::current_exception());
    }
}


- (void)sendRunEvent {
    try {
        self.client->sendRunEvent();
    } catch (...) {
        MWorksSwiftLogException(std::current_exception());
    }
}


- (void)sendStopEvent {
    try {
        self.client->sendStopEvent();
    } catch (...) {
        MWorksSwiftLogException(std::current_exception());
    }
}


- (void)sendPauseEvent {
    try {
        self.client->sendPauseEvent();
    } catch (...) {
        MWorksSwiftLogException(std::current_exception());
    }
}


- (void)sendResumeEvent {
    try {
        self.client->sendResumeEvent();
    } catch (...) {
        MWorksSwiftLogException(std::current_exception());
    }
}


- (void)sendSaveVariablesEvent:(NSString *)variableSetName overwrite:(BOOL)overwrite {
    try {
        self.client->sendSaveVariablesEvent(variableSetName.UTF8String, overwrite);
    } catch (...) {
        MWorksSwiftLogException(std::current_exception());
    }
}


- (void)sendLoadVariablesEvent:(NSString *)variableSetName {
    try {
        self.client->sendLoadVariablesEvent(variableSetName.UTF8String);
    } catch (...) {
        MWorksSwiftLogException(std::current_exception());
    }
}


- (void)sendOpenDataFileEvent:(NSString *)filename overwrite:(BOOL)overwrite {
    try {
        self.client->sendOpenDataFileEvent(filename.UTF8String, overwrite);
    } catch (...) {
        MWorksSwiftLogException(std::current_exception());
    }
}


- (void)sendCloseDataFileEvent:(NSString *)filename {
    try {
        self.client->sendCloseDataFileEvent(filename.UTF8String);
    } catch (...) {
        MWorksSwiftLogException(std::current_exception());
    }
}


@end
