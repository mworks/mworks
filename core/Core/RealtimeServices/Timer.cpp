/*
 *  untitled.cpp
 *  ExperimentControlCocoa
 *
 *  Created by David Cox on 10/12/04.
 *  Copyright 2004 __MyCompanyName__. All rights reserved.
 *
 */

#include "Timer.h"
#include "boost/bind.hpp"


BEGIN_NAMESPACE_MW


TimeBase::TimeBase() {
	setNow();
}


void TimeBase::setTime(MWTime _time){
	time_us = _time;
}


void TimeBase::setNow(){
	time_us = Clock::instance()->getCurrentTimeUS();
}


MWTime TimeBase::getTime() const {
	return time_us;
}


Timer::Timer(VariableProperties *props) :
    ReadOnlyVariable(props),
    clock(Clock::instance()),
    expirationTimeUS(0)
{ }


void Timer::startMS(MWTime howlongms) {
	startUS(howlongms * 1000);
}


void Timer::startUS(MWTime howlongus) {
	scoped_lock lock(mutex);
	
	if (howlongus <= 0) {
		mwarning(M_SYSTEM_MESSAGE_DOMAIN, "Scheduling a timer to fire at a time in the past");
	}
    
    expirationTimeUS = clock->getCurrentTimeUS() + howlongus;
}


bool Timer::hasExpired() const {
	scoped_lock lock(mutex);
	return (clock->getCurrentTimeUS() >= expirationTimeUS);
}


Datum Timer::getValue() {
	return Datum(hasExpired());
}


END_NAMESPACE_MW



























