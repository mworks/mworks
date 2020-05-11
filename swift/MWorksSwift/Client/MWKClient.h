//
//  MWKClient.h
//  MWorksSwift
//
//  Created by Christopher Stawarz on 9/11/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#import <Foundation/Foundation.h>

#import <MWorksSwift/MWorksSwiftDefines.h>
#import <MWorksSwift/MWKCore.h>


NS_ASSUME_NONNULL_BEGIN


@class MWKDatum;


@protocol MWKClient <MWKCore>

- (nullable MWKDatum *)valueForCode:(NSInteger)code;
- (nullable MWKDatum *)valueForTag:(NSString *)tag;
- (BOOL)setValue:(MWKDatum *)value forCode:(NSInteger)code;
- (BOOL)setValue:(MWKDatum *)value forTag:(NSString *)tag;

- (void)logMessage:(NSString *)message;
- (void)logWarning:(NSString *)warning;
- (void)logError:(NSString *)error;

@end


MWORKSSWIFT_PUBLIC_CLASS
@interface MWKClient : MWKCore <MWKClient>

+ (nullable instancetype)clientWithError:(NSError * _Nullable *)error;

- (void)startEventListener;

@property(nonatomic, readonly) BOOL connected;
- (BOOL)connectToServerAtAddress:(NSString *)address port:(NSInteger)port;
- (BOOL)disconnect;

- (BOOL)sendExperimentAtPath:(NSString *)path;
- (void)sendCloseExperimentEventWithPath:(NSString *)path;

- (void)sendProtocolSelectedEventWithProtocolName:(NSString *)protocolName;

- (void)sendRunEvent;
- (void)sendStopEvent;
- (void)sendPauseEvent;
- (void)sendResumeEvent;

- (void)sendSaveVariablesEventWithVariableSetName:(NSString *)variableSetName overwrite:(BOOL)overwrite;
- (void)sendLoadVariablesEventWithVariableSetName:(NSString *)variableSetName;

- (void)sendOpenDataFileEventWithFilename:(NSString *)filename overwrite:(BOOL)overwrite;
- (void)sendCloseDataFileEventWithFilename:(NSString *)filename;

@end


NS_ASSUME_NONNULL_END
