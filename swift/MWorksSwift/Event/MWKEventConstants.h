//
//  MWKEventConstants.h
//  MWorksSwift
//
//  Created by Christopher Stawarz on 9/9/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#import <Foundation/Foundation.h>

#import <MWorksSwift/MWorksSwiftDefines.h>


NS_ASSUME_NONNULL_BEGIN


typedef NSInteger MWKReservedEventCode NS_TYPED_ENUM;
MWORKSSWIFT_PUBLIC_EXTERN MWKReservedEventCode const MWKReservedEventCodeSystemEvent;


typedef NSString * MWKSystemEventKey NS_TYPED_ENUM;
MWORKSSWIFT_PUBLIC_EXTERN MWKSystemEventKey const MWKSystemEventKeyPayloadType;
MWORKSSWIFT_PUBLIC_EXTERN MWKSystemEventKey const MWKSystemEventKeyPayload;


typedef NSInteger MWKSystemEventPayloadType NS_TYPED_ENUM;
MWORKSSWIFT_PUBLIC_EXTERN MWKSystemEventPayloadType const MWKSystemEventPayloadTypeExperimentState;


typedef NSString * MWKSystemEventPayloadKey NS_TYPED_ENUM;
MWORKSSWIFT_PUBLIC_EXTERN MWKSystemEventPayloadKey const MWKSystemEventPayloadKeyLoaded;
MWORKSSWIFT_PUBLIC_EXTERN MWKSystemEventPayloadKey const MWKSystemEventPayloadKeyRunning;


NS_ASSUME_NONNULL_END
