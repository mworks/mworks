/*
 *  Clock.cpp
 *  Experimental Control with Cocoa UI
 *
 *  Created by David Cox on Sun Dec 29 2002.
 *  Copyright (c) 2002 __MyCompanyName__. All rights reserved.
 *
 */

#include "Clock.h"
using namespace mw;

//Clock::singleton_instance = shared_ptr<Clock>();

//template <> shared_ptr<Clock> RegisteredSingleton<Clock>::singleton_instance = shared_ptr<Clock>();
SINGLETON_INSTANCE_STATIC_DECLARATION(Clock)

Clock::Clock(long interval_microseconds){ }
Clock::~Clock() { }

MWTime Clock::getCurrentTime(){
    return getCurrentTimeMS();
}

MWTime Clock::getCurrentTimeMS(){
    return 0;
}
MWTime Clock::getCurrentTimeUS(){
    return 0;
}
MWTime Clock::getCurrentTimeNS(){
    return 0;
}



void Clock::sleepMS(MWTime time){ return; }
void Clock::sleepUS(MWTime time){ return; }
void Clock::sleepNS(MWTime time){ return; }

long Clock::getInterval(){
    return 0;
}

void Clock::setInterval(long micro){ }

void Clock::startClock(){ }
void Clock::stopClock(){ }

MWTime Clock::getSystemTimeMS(){
    return 0;
}

MWTime Clock::getSystemTimeUS(){
    return 0;
}

MWTime Clock::getSystemTimeNS(){
    return 0;
}

MWTime Clock::getSystemBaseTimeNS(){
    return 0;
}


MWTime Clock::getSystemReferenceTime() {
//    return 0;
    return getSystemBaseTimeNS() / 1000;
}
