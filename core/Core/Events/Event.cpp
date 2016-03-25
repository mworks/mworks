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


Event::Event(int code, const Datum &data) :
    Event(code, Clock::instance()->getCurrentTimeUS(), data)
{ }


END_NAMESPACE_MW
