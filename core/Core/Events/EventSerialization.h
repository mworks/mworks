/*
 *  EventSerialization.h
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 8/5/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */

#include "Event.h"
#include "ScarabServices.h"

Event *scarabDatumToEvent(ScarabDatum *datum);
ScarabDatum *eventToScarabDatum(Event *event);