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

//mRegisteredSingleton<Clock>::singleton_instance = shared_ptr<Clock>();

template <> shared_ptr<Clock> RegisteredSingleton<Clock>::singleton_instance = shared_ptr<Clock>();


Clock::Clock(long interval_microseconds){ }
Clock::~Clock() { }

MonkeyWorksTime Clock::getCurrentTime(){
    return getCurrentTimeMS();
}

MonkeyWorksTime Clock::getCurrentTimeMS(){
    return 0;
}
MonkeyWorksTime Clock::getCurrentTimeUS(){
    return 0;
}
MonkeyWorksTime Clock::getCurrentTimeNS(){
    return 0;
}



void Clock::sleepMS(MonkeyWorksTime time){ return; }
void Clock::sleepUS(MonkeyWorksTime time){ return; }
void Clock::sleepNS(MonkeyWorksTime time){ return; }

long Clock::getInterval(){
    return 0;
}

void Clock::setInterval(long micro){ }

void Clock::startClock(){ }
void Clock::stopClock(){ }

MonkeyWorksTime Clock::getSystemTimeMS(){
    return 0;
}

MonkeyWorksTime Clock::getSystemTimeUS(){
    return 0;
}

MonkeyWorksTime Clock::getSystemTimeNS(){
    return 0;
}


MonkeyWorksTime Clock::getSystemReferenceTime() {
    return 0;
}
