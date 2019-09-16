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


MWORKSSWIFT_PUBLIC_CLASS
@interface MWKClient : MWKCore

+ (BOOL)constructCore:(NSError * _Nullable *)error;

+ (nullable instancetype)client:(NSError * _Nullable *)error;

- (void)startEventListener;

@property(nonatomic, readonly) BOOL connected;
- (BOOL)connectToServer:(NSString *)address port:(NSInteger)port;
- (BOOL)disconnect;

- (BOOL)sendExperiment:(NSString *)path;
- (void)sendCloseExperimentEvent:(NSString *)path;

- (void)sendProtocolSelectedEvent:(NSString *)protocolName;

- (void)sendRunEvent;
- (void)sendStopEvent;
- (void)sendPauseEvent;
- (void)sendResumeEvent;

- (void)sendSaveVariablesEvent:(NSString *)variableSetName overwrite:(BOOL)overwrite;
- (void)sendLoadVariablesEvent:(NSString *)variableSetName;

- (void)sendOpenDataFileEvent:(NSString *)filename overwrite:(BOOL)overwrite;
- (void)sendCloseDataFileEvent:(NSString *)filename;

- (nullable MWKDatum *)valueForTag:(NSString *)tag;
- (BOOL)setValue:(MWKDatum *)value forTag:(NSString *)tag;

@end


NS_ASSUME_NONNULL_END
