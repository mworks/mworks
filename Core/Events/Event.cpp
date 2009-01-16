/**
 * Event.cpp
 *
 * History:
 * David Cox on Thu Dec 05 2002 - Created.
 * Paul Jankunas on 12/20/04 - Added NetworkEvent class.
 *
 * Copyright (c) 2002 MIT. All rights reserved.
 */

#include "Event.h"
#include "Utilities.h"
#include "VariableRegistry.h"
#include "ScarabServices.h"
using namespace mw;

// -----------------------------------------------------------------
//  Event Methods
// -----------------------------------------------------------------	  
Event::Event(const int _code, 
			 const MonkeyWorksTime _time, 
			 const Data &_data) {
	code = _code;
	data = _data; 
	time = _time;
	nextEvent = shared_ptr<Event>();
}

Event::Event(const int _code, 
			 const Data &_data) {
	code = _code;
	data = _data; 
	shared_ptr <Clock> clock = Clock::instance();
	time = clock->getCurrentTimeUS();
	nextEvent = shared_ptr<Event>();
}


Event::Event(ScarabDatum *datum) {  // create an event from a ScarabDatum
	
	if(datum->type != SCARAB_LIST){   // is this a list (as it should be)?
		// TODO: warn here
		mwarning(M_NETWORK_MESSAGE_DOMAIN,
				 "Attempt to create an event object from an invalid ScarabDatum");
		time = 0L;
		code = -1;
		data = Data();
		return;
	}
	
	ScarabDatum *code_datum = scarab_list_get(datum, SCARAB_EVENT_CODEC_CODE_INDEX);
	if(code_datum->type != SCARAB_INTEGER){	// is the code an integer?
		// TODO: warn here
		mwarning(M_NETWORK_MESSAGE_DOMAIN,
				 "Attempt to create an event object with an invalid code datum (%d)",
				 code_datum->type);
		time = 0L;
		code = -1;
		data = Data();
		return;
	}
	code = code_datum->data.integer;
	
	
	ScarabDatum *time_datum = scarab_list_get(datum, SCARAB_EVENT_TIME_INDEX);
	if(time_datum == NULL){
		mwarning(M_NETWORK_MESSAGE_DOMAIN,
				 "Bad time datum received over the network (NULL)");
		time = 0L;
	} else if(time_datum->type != SCARAB_INTEGER){
		mwarning(M_NETWORK_MESSAGE_DOMAIN,
				 "Bad time datum received over the network (invalid type: %d)",
				 time_datum->type);
		time = 0L;
	} else {
		time = time_datum->data.integer;
	}
	
	ScarabDatum *payload = scarab_list_get(datum, SCARAB_EVENT_PAYLOAD_INDEX);
	
	data = Data(payload);	
	nextEvent = shared_ptr<Event>();
}

ScarabDatum *Event::toScarabDatum(){
	boost::mutex::scoped_lock lock(eventLock);	
	
	ScarabDatum *event_datum = scarab_list_new(SCARAB_PAYLOAD_EVENT_N_TOPLEVEL_ELEMENTS);
	
	
	ScarabDatum *code_datum = scarab_new_integer(code);
	ScarabDatum *time_datum = scarab_new_integer(time);
	
	scarab_list_put(event_datum, 
					SCARAB_EVENT_CODEC_CODE_INDEX, 
					code_datum);
	
	scarab_list_put(event_datum, 
					SCARAB_EVENT_TIME_INDEX, 
					time_datum);
 	scarab_list_put(event_datum, 
					SCARAB_EVENT_PAYLOAD_INDEX, 
					data.getScarabDatum());
	
	scarab_free_datum(code_datum);
	scarab_free_datum(time_datum);
	
	return event_datum;   
}
