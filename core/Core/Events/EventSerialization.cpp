/*
 *  EventSerialization.cpp
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 8/5/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */

#ifdef	SHOULD_THIS_EVEN_BE_IN_HERE

#include "EventSerialization.h"
#include "VariableRegistry.h"
#include "Utilities.h"
#include "Event.h"
#include "EventBuffer.h"


ScarabDatum *eventToScarabDatum(Event *event){ 
	// todo
	return NULL; 	
};

Event *scarabDatumToEvent(ScarabDatum *datum){

	ScarabDatum * info;
	switch(datum->type) {
		case 0://SCARAB_NULL
		case 1://SCARAB_INTEGER
		case 2://SCARAB_FLOAT
		case 3://SCARAB_FLOAT_INF
		case 4://SCARAB_FLOAT+NAN
			break;
		case 6://SCARAB_LIST
			//mdebug("Received List");
			info = scarab_list_get(datum, 0); // the codec code.
			//mdebug("List type is (%d)", info->type);
			if(info == NULL || info->type != SCARAB_INTEGER) {
				// bad list!
				mdebug("Invalid or NULL list received");
				break;
			}
			if(info->data.integer == _MESSAGE_CODE) {
				//message event
				mdebug("Received message event");
				info = scarab_list_get(datum, 2);
				if(info->type != SCARAB_DICT) { break; }
				MessageEvent * m = new MessageEvent(info);
				GlobalBufferManager->putIncomingNetworkEvent(m);
			} else if(info->data.integer == _TERMINATION_CODE) {
				//termination event
				mdebug("Received Termination code");
				break;
			} else if(info->data.integer == _SCARAB_PACKAGE_CODE) {
				mdebug("Received package event");
				info = scarab_list_get(datum, 2);
				SystemEvent * e = new SystemEvent(info);
				//mScarabPackageEvent * e = new ScarabPackageEvent(info);
				//GlobalBufferManager->putIncomingNetworkEvent(e);
				return (Event *)e;
			} else if(info->data.integer > _CODEC_MAX) {
				//data event
				mdebug("got a data event");
				int code = info->data.integer; //codec code
				mdebug("codec code is %d",  code);
				info = scarab_list_get(datum, 1); // the time
				int time = info->data.integer; // do something with this
				
				info = scarab_list_get(datum, 2); //data type
				int type = info->data.integer;
				mdebug("Got data type %d", type);
				info = scarab_list_get(datum, 3); // the data
				DataEvent * g = new DataEvent(code, info, type);
				//mGenericEvent * g = new GenericEvent(code, info, type);
				return (Event *)g;
			}
			break;
		case 7://SCARAB_OPAQUE
			break;
		case 5://SCARAB_DICT
			break;
		default:
			mdebug("Bad Data Type Received From Scarab Stream!");
			break;
	}
	
	return NULL;
}

#endif