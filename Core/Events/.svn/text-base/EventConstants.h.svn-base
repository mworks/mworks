/*
 *  EventConstants.h
 *  MonkeyWorksCore
 *
 *  Created by Ben Kennedy on 10/6/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _EVENT_CONSTANTS_H
#define _EVENT_CONSTANTS_H

#define RESERVED_CODEC_CODE 0
#define RESERVED_COMPONENT_CODEC_CODE 1 

#define SCARAB_PAYLOAD_EVENT_N_TOPLEVEL_ELEMENTS	3
#define SCARAB_EVENT_N_TOPLEVEL_ELEMENTS			2

#define SCARAB_EVENT_CODEC_CODE_INDEX	0
#define SCARAB_EVENT_TIME_INDEX			1
#define SCARAB_EVENT_PAYLOAD_INDEX		2

#define M_SYSTEM_EVENT_TYPE "event_type"
#define M_SYSTEM_PAYLOAD_TYPE "payload_type"
#define M_SYSTEM_PAYLOAD "payload"

#define SCARAB_SYSTEM_EVENT_EVENT_TYPE_INDEX            0
#define SCARAB_SYSTEM_EVENT_PAYLOAD_TYPE_INDEX          1
#define SCARAB_SYSTEM_EVENT_PAYLOAD_INDEX               2
#define SCARAB_SYSTEM_EVENT_N_PAYLOAD_ELEMENTS          3
namespace mw {
// this describes the high level structure of the system event
typedef enum {
    M_SYSTEM_CONTROL_PACKAGE        = 1000, 
    M_SYSTEM_DATA_PACKAGE           = 1001, 
    M_SYSTEM_RESPONSE_PACKAGE       = 1002,
} SystemEventType;

// the EventFactory should be used to code and decode all of these messages.
typedef enum {
	// data package types whose payloads are data
    M_EXPERIMENT_PACKAGE                = 2000,
    M_PROTOCOL_PACKAGE                  = 2001,
    M_DATAFILE_PACKAGE                  = 2004, //the actual file
												// control events that tell the server or client to perform a task
												// they usually have no payload (mw::Protocol Selection contains the name
												// of the protocol selected)
    M_PROTOCOL_SELECTION                = 3001,
    M_START_EXPERIMENT                  = 3002,
    M_STOP_EXPERIMENT                   = 3003,
    M_PAUSE_EXPERIMENT                  = 3004,
    M_OPEN_DATA_FILE                    = 3005, //the open not the actual file.
    M_CLOSE_DATA_FILE                   = 3006,
    M_CLOSE_EXPERIMENT                  = 3007,
    M_SAVE_VARIABLES                    = 3008,
    M_LOAD_VARIABLES                    = 3009,
	// response messages
    M_DATA_FILE_OPENED                  = 4007,
    M_DATA_FILE_CLOSED                  = 4008,
    M_CLIENT_CONNECTED_TO_SERVER        = 4009,
    M_CLIENT_DISCONNECTED_FROM_SERVER   = 4010,
    M_SERVER_CONNECTED_CLIENT           = 4011,
    M_SERVER_DISCONNECTED_CLIENT        = 4012,
	M_EXPERIMENT_STATE					= 4013,
	// this payload can contain whatever
    M_USER_DEFINED                      = 6000,
} SystemPayloadType;

// used in the event factory
typedef enum {
    M_COMMAND_SUCCESS               = 5001,
    M_COMMAND_FAILURE               = 5002,
} SystemEventResponseCode;



enum MessageType { M_INVALID_MESSAGE		= -1, 
	M_GENERIC_MESSAGE		= 0,
	M_WARNING_MESSAGE		= 1,
	M_ERROR_MESSAGE			= 2,
	M_FATAL_ERROR_MESSAGE	= 3,
	M_MAX_MESSAGE_TYPE
};

enum MessageDomain { M_GENERIC_MESSAGE_DOMAIN		= 0, 
	M_NETWORK_MESSAGE_DOMAIN		= 1,
	M_PARSER_MESSAGE_DOMAIN			= 2,
	M_PARADIGM_MESSAGE_DOMAIN		= 3,
	M_SCHEDULER_MESSAGE_DOMAIN		= 4,
	M_STATE_SYSTEM_MESSAGE_DOMAIN  = 5,
	M_DISPLAY_MESSAGE_DOMAIN		= 6,
	M_IODEVICE_MESSAGE_DOMAIN		= 7,
	M_FILE_MESSAGE_DOMAIN			= 8,
	M_SYSTEM_MESSAGE_DOMAIN			= 9,
	M_PLUGIN_MESSAGE_DOMAIN			= 10,
	M_CLIENT_MESSAGE_DOMAIN			= 11,
	M_SERVER_MESSAGE_DOMAIN			= 12
};


#define M_MESSAGE_DOMAIN "domain"
#define M_MESSAGE "message"
#define M_MESSAGE_TYPE "type"
#define M_MESSAGE_ORIGIN "origin"

#define M_HOST_NAME "hostname"
#define M_PORT "port"
#define M_EXPERIMENT_NAME "experiment name"
#define M_EXPERIMENT_PATH "experiment path"
#define M_PROTOCOL_NAME "protocol name"
#define M_LOADED "loaded"
#define M_RUNNING "running"
#define M_PAUSED "paused"
#define M_DESCRIPTION "description"
#define M_PROTOCOLS "protocols"
#define M_CURRENT_PROTOCOL "current protocol"
#define M_SAVED_VARIABLES "saved variables"

#define SAVED_VARIABLES_DIR_NAME "SavedVariables"


//enum EventType{	M_UNDEFINED_EVENT = 1000,
//	M_SYSTEM_EVENT =	1001,
//	M_DATA_EVENT =		1002 
//};


// TODO: this is clumsy
enum EventCode {	M_UNDEFINED_EVENT_CODE =		-999,
	M_TERMINATION_EVENT_CODE =		2,
	M_MAX_RESERVED_EVENT_CODE = 3
};

enum MessageOrigin {
	M_UNKNOWN_MESSAGE_ORIGIN = 0,
	M_SERVER_MESSAGE_ORIGIN = 1,
	M_CLIENT_MESSAGE_ORIGIN = 2,	
};


// TODO: clean this up														 			 
#define M_DATA_EVENT_MIN_CODE	M_MAX_RESERVED_EVENT_CODE
}
#endif


