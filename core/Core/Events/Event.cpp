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


BEGIN_NAMESPACE_MW


// -----------------------------------------------------------------
//  Event Methods
// -----------------------------------------------------------------	  
Event::Event(const int _code, 
			 const MWTime _time, 
			 const Datum &_data) {
	code = _code;
	data = _data; 
	time = _time;
	nextEvent = shared_ptr<Event>();
}

Event::Event(const int _code, 
			 const Datum &_data) {
	code = _code;
	data = _data; 
	shared_ptr <Clock> clock = Clock::instance();
	time = clock->getCurrentTimeUS();
	nextEvent = shared_ptr<Event>();
}


END_NAMESPACE_MW
