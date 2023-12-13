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


typedef NSInteger MWKReservedEventCode NS_TYPED_ENUM NS_SWIFT_NAME(ReservedEventCode);
MWORKSSWIFT_PUBLIC_EXTERN MWKReservedEventCode const MWKReservedEventCodeCodec;
MWORKSSWIFT_PUBLIC_EXTERN MWKReservedEventCode const MWKReservedEventCodeSystemEvent;
MWORKSSWIFT_PUBLIC_EXTERN MWKReservedEventCode const MWKReservedEventCodeComponentCodec;
MWORKSSWIFT_PUBLIC_EXTERN MWKReservedEventCode const MWKReservedEventCodeTermination;


typedef NSString * MWKSystemEventKey NS_TYPED_ENUM NS_SWIFT_NAME(SystemEventKey);
MWORKSSWIFT_PUBLIC_EXTERN MWKSystemEventKey const MWKSystemEventKeyPayloadType;
MWORKSSWIFT_PUBLIC_EXTERN MWKSystemEventKey const MWKSystemEventKeyPayload;


typedef NSInteger MWKSystemEventPayloadType NS_TYPED_ENUM NS_SWIFT_NAME(SystemEventPayloadType);
MWORKSSWIFT_PUBLIC_EXTERN MWKSystemEventPayloadType const MWKSystemEventPayloadTypeProtocolPackage;
MWORKSSWIFT_PUBLIC_EXTERN MWKSystemEventPayloadType const MWKSystemEventPayloadTypeDataFileOpened;
MWORKSSWIFT_PUBLIC_EXTERN MWKSystemEventPayloadType const MWKSystemEventPayloadTypeDataFileClosed;
MWORKSSWIFT_PUBLIC_EXTERN MWKSystemEventPayloadType const MWKSystemEventPayloadTypeExperimentState;


typedef NSInteger MWKSystemEventResponseCode NS_TYPED_ENUM NS_SWIFT_NAME(SystemEventResponseCode);
MWORKSSWIFT_PUBLIC_EXTERN MWKSystemEventResponseCode const MWKSystemEventResponseCodeCommandSuccess;
MWORKSSWIFT_PUBLIC_EXTERN MWKSystemEventResponseCode const MWKSystemEventResponseCodeCommandFailure;


typedef NSString * MWKSystemEventPayloadKey NS_TYPED_ENUM NS_SWIFT_NAME(SystemEventPayloadKey);
MWORKSSWIFT_PUBLIC_EXTERN MWKSystemEventPayloadKey const MWKSystemEventPayloadKeyExperimentName;
MWORKSSWIFT_PUBLIC_EXTERN MWKSystemEventPayloadKey const MWKSystemEventPayloadKeyExperimentPath;
MWORKSSWIFT_PUBLIC_EXTERN MWKSystemEventPayloadKey const MWKSystemEventPayloadKeyProtocolName;
MWORKSSWIFT_PUBLIC_EXTERN MWKSystemEventPayloadKey const MWKSystemEventPayloadKeyLoaded;
MWORKSSWIFT_PUBLIC_EXTERN MWKSystemEventPayloadKey const MWKSystemEventPayloadKeyRunning;
MWORKSSWIFT_PUBLIC_EXTERN MWKSystemEventPayloadKey const MWKSystemEventPayloadKeyPaused;
MWORKSSWIFT_PUBLIC_EXTERN MWKSystemEventPayloadKey const MWKSystemEventPayloadKeyProtocols;
MWORKSSWIFT_PUBLIC_EXTERN MWKSystemEventPayloadKey const MWKSystemEventPayloadKeyCurrentProtocolName;
MWORKSSWIFT_PUBLIC_EXTERN MWKSystemEventPayloadKey const MWKSystemEventPayloadKeySavedVariableSetNames;
MWORKSSWIFT_PUBLIC_EXTERN MWKSystemEventPayloadKey const MWKSystemEventPayloadKeyCurrentSavedVariableSetName;
MWORKSSWIFT_PUBLIC_EXTERN MWKSystemEventPayloadKey const MWKSystemEventPayloadKeyDataFileAutoOpen;
MWORKSSWIFT_PUBLIC_EXTERN MWKSystemEventPayloadKey const MWKSystemEventPayloadKeyDataFileName;


typedef NSString * MWKMessageKey NS_TYPED_ENUM NS_SWIFT_NAME(MessageKey);
MWORKSSWIFT_PUBLIC_EXTERN MWKMessageKey const MWKMessageKeyDomain;
MWORKSSWIFT_PUBLIC_EXTERN MWKMessageKey const MWKMessageKeyMessage;
MWORKSSWIFT_PUBLIC_EXTERN MWKMessageKey const MWKMessageKeyType;
MWORKSSWIFT_PUBLIC_EXTERN MWKMessageKey const MWKMessageKeyOrigin;


typedef NSInteger MWKMessageType NS_TYPED_ENUM NS_SWIFT_NAME(MessageType);
MWORKSSWIFT_PUBLIC_EXTERN MWKMessageType const MWKMessageTypeGeneric;
MWORKSSWIFT_PUBLIC_EXTERN MWKMessageType const MWKMessageTypeWarning;
MWORKSSWIFT_PUBLIC_EXTERN MWKMessageType const MWKMessageTypeError;


NS_ASSUME_NONNULL_END
