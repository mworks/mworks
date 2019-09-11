//
//  MWKServer.h
//  MWorksSwift
//
//  Created by Christopher Stawarz on 9/5/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#import <Foundation/Foundation.h>

#import <MWorksSwift/MWorksSwiftDefines.h>
#import <MWorksSwift/MWKCore.h>


NS_ASSUME_NONNULL_BEGIN


MWORKSSWIFT_PUBLIC_CLASS
@interface MWKServer : MWKCore

+ (BOOL)constructCore:(NSError * _Nullable *)error;

@property(class, nonatomic, readonly) NSString *hostName;

+ (nullable instancetype)serverWithListeningAddress:(NSString *)address
                                      listeningPort:(NSInteger)port
                                              error:(NSError * _Nullable *)error;

- (BOOL)start;
- (BOOL)stop;

- (BOOL)openExperiment:(NSString *)path;
- (BOOL)closeExperiment;

- (void)startExperiment;

@end


NS_ASSUME_NONNULL_END
