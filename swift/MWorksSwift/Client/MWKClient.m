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
#import "MWKDatum_Private.h"


@implementation MWKClient {
    boost::shared_ptr<mw::Client> _client;
}


+ (instancetype)clientWithError:(NSError **)error {
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


- (BOOL)connectToServerAtAddress:(NSString *)address port:(NSInteger)port {
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


- (BOOL)sendExperimentAtPath:(NSString *)path {
    BOOL result = NO;
    try {
        result = self.client->sendExperiment(path.UTF8String);
    } catch (...) {
        MWorksSwiftLogException(std::current_exception());
    }
    return result;
}


- (void)sendCloseExperimentEventWithPath:(NSString *)path {
    try {
        self.client->sendCloseExperimentEvent(path.UTF8String);
    } catch (...) {
        MWorksSwiftLogException(std::current_exception());
    }
}


- (void)sendProtocolSelectedEventWithProtocolName:(NSString *)protocolName {
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


- (void)sendSaveVariablesEventWithVariableSetName:(NSString *)variableSetName overwrite:(BOOL)overwrite {
    try {
        self.client->sendSaveVariablesEvent(variableSetName.UTF8String, overwrite);
    } catch (...) {
        MWorksSwiftLogException(std::current_exception());
    }
}


- (void)sendLoadVariablesEventWithVariableSetName:(NSString *)variableSetName {
    try {
        self.client->sendLoadVariablesEvent(variableSetName.UTF8String);
    } catch (...) {
        MWorksSwiftLogException(std::current_exception());
    }
}


- (void)sendOpenDataFileEventWithFilename:(NSString *)filename overwrite:(BOOL)overwrite {
    try {
        self.client->sendOpenDataFileEvent(filename.UTF8String, overwrite);
    } catch (...) {
        MWorksSwiftLogException(std::current_exception());
    }
}


- (void)sendCloseDataFileEventWithFilename:(NSString *)filename {
    try {
        self.client->sendCloseDataFileEvent(filename.UTF8String);
    } catch (...) {
        MWorksSwiftLogException(std::current_exception());
    }
}


- (MWKDatum *)valueForTag:(NSString *)tag {
    MWKDatum *result = nil;
    try {
        if (auto var = self.client->getVariable(tag.UTF8String)) {
            result = [MWKDatum datumWithDatum:var->getValue()];
        }
    } catch (...) {
        MWorksSwiftLogException(std::current_exception());
    }
    return result;
}


- (BOOL)setValue:(MWKDatum *)value forTag:(NSString *)tag {
    BOOL result = NO;
    NSInteger code = [self codeForTag:tag];
    if (code >= 0) {
        try {
            self.client->updateValue(code, value.datum);
            result = YES;
        } catch (...) {
            MWorksSwiftLogException(std::current_exception());
        }
    }
    return result;
}


@end
