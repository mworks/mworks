//
//  MWKEventConstants.m
//  MWorksSwift
//
//  Created by Christopher Stawarz on 9/9/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#import "MWKEventConstants.h"


MWKReservedEventCode const MWKReservedEventCodeCodec = mw::RESERVED_CODEC_CODE;
MWKReservedEventCode const MWKReservedEventCodeSystemEvent = mw::RESERVED_SYSTEM_EVENT_CODE;
MWKReservedEventCode const MWKReservedEventCodeComponentCodec = mw::RESERVED_COMPONENT_CODEC_CODE;
MWKReservedEventCode const MWKReservedEventCodeTermination = mw::RESERVED_TERMINATION_CODE;


MWKSystemEventKey const MWKSystemEventKeyPayloadType = @M_SYSTEM_PAYLOAD_TYPE;
MWKSystemEventKey const MWKSystemEventKeyPayload = @M_SYSTEM_PAYLOAD;


MWKSystemEventPayloadType const MWKSystemEventPayloadTypeProtocolPackage = mw::M_PROTOCOL_PACKAGE;
MWKSystemEventPayloadType const MWKSystemEventPayloadTypeDataFileOpened = mw::M_DATA_FILE_OPENED;
MWKSystemEventPayloadType const MWKSystemEventPayloadTypeDataFileClosed = mw::M_DATA_FILE_CLOSED;
MWKSystemEventPayloadType const MWKSystemEventPayloadTypeExperimentState = mw::M_EXPERIMENT_STATE;


MWKSystemEventResponseCode const MWKSystemEventResponseCodeCommandSuccess = mw::M_COMMAND_SUCCESS;
MWKSystemEventResponseCode const MWKSystemEventResponseCodeCommandFailure = mw::M_COMMAND_FAILURE;


MWKSystemEventPayloadKey const MWKSystemEventPayloadKeyExperimentName = @M_EXPERIMENT_NAME;
MWKSystemEventPayloadKey const MWKSystemEventPayloadKeyExperimentPath = @M_EXPERIMENT_PATH;
MWKSystemEventPayloadKey const MWKSystemEventPayloadKeyProtocolName = @M_PROTOCOL_NAME;
MWKSystemEventPayloadKey const MWKSystemEventPayloadKeyLoaded = @M_LOADED;
MWKSystemEventPayloadKey const MWKSystemEventPayloadKeyRunning = @M_RUNNING;
MWKSystemEventPayloadKey const MWKSystemEventPayloadKeyPaused = @M_PAUSED;
MWKSystemEventPayloadKey const MWKSystemEventPayloadKeyProtocols = @M_PROTOCOLS;
MWKSystemEventPayloadKey const MWKSystemEventPayloadKeyCurrentProtocolName = @M_CURRENT_PROTOCOL;
MWKSystemEventPayloadKey const MWKSystemEventPayloadKeySavedVariableSetNames = @M_SAVED_VARIABLES;
MWKSystemEventPayloadKey const MWKSystemEventPayloadKeyCurrentSavedVariableSetName = @M_CURRENT_SAVED_VARIABLES;
MWKSystemEventPayloadKey const MWKSystemEventPayloadKeyDataFileAutoOpen = @M_DATA_FILE_AUTO_OPEN;
MWKSystemEventPayloadKey const MWKSystemEventPayloadKeyDataFileName = @M_DATA_FILE_NAME;


MWKMessageKey const MWKMessageKeyDomain = @M_MESSAGE_DOMAIN;
MWKMessageKey const MWKMessageKeyMessage = @M_MESSAGE;
MWKMessageKey const MWKMessageKeyType = @M_MESSAGE_TYPE;
MWKMessageKey const MWKMessageKeyOrigin = @M_MESSAGE_ORIGIN;


MWKMessageType const MWKMessageTypeGeneric = mw::M_GENERIC_MESSAGE;
MWKMessageType const MWKMessageTypeWarning = mw::M_WARNING_MESSAGE;
MWKMessageType const MWKMessageTypeError = mw::M_ERROR_MESSAGE;
