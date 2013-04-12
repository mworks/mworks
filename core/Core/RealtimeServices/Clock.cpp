/*
 *  Clock.cpp
 *  Experimental Control with Cocoa UI
 *
 *  Created by David Cox on Sun Dec 29 2002.
 *  Copyright (c) 2002 __MyCompanyName__. All rights reserved.
 *
 */

#include "Clock.h"


BEGIN_NAMESPACE_MW


SINGLETON_INSTANCE_STATIC_DECLARATION(Clock)


MWTime Clock::getCurrentTimeNS() {
    return 0;
}


void Clock::sleepNS(MWTime time) { }


MWTime Clock::getSystemTimeNS() {
    return 0;
}


MWTime Clock::getSystemBaseTimeNS() {
    return 0;
}


END_NAMESPACE_MW
