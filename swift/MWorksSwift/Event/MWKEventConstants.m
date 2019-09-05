//
//  MWKEventConstants.m
//  MWorksSwift
//
//  Created by Christopher Stawarz on 9/9/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#import "MWKEventConstants.h"


MWKReservedEventCode const MWKReservedEventCodeSystemEvent = mw::RESERVED_SYSTEM_EVENT_CODE;


MWKSystemEventKey const MWKSystemEventKeyPayloadType = @M_SYSTEM_PAYLOAD_TYPE;
MWKSystemEventKey const MWKSystemEventKeyPayload = @M_SYSTEM_PAYLOAD;


MWKSystemEventPayloadType const MWKSystemEventPayloadTypeExperimentState = mw::M_EXPERIMENT_STATE;


MWKSystemEventPayloadKey const MWKSystemEventPayloadKeyLoaded = @M_LOADED;
MWKSystemEventPayloadKey const MWKSystemEventPayloadKeyRunning = @M_RUNNING;
