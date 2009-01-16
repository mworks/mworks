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
using namespace mw;


TimeBase::TimeBase() {
	setNow();
}

void TimeBase::setTime(MonkeyWorksTime _time){
	time_us = _time;
}
void TimeBase::setNow(){
	shared_ptr <Clock> clock = Clock::instance();
	time_us = clock->getCurrentTimeUS();
}

MonkeyWorksTime TimeBase::getTime(){
	return time_us;
}



Timer::Timer(VariableProperties *props) : Variable(props) {
	internalLock = shared_ptr<boost::mutex>(new boost::mutex());
	//schedule_node_lock = shared_ptr<boost::mutex>(new boost::mutex());
	has_expired = shared_ptr<bool>(new bool(true));
}

Timer::~Timer(){ }

void Timer::start(MonkeyWorksTime howlongms){
	startMS(howlongms);
}

void Timer::startMS(MonkeyWorksTime howlongms){
	
	startUS(howlongms * 1000);
}

void Timer::forceExpired(){
	boost::mutex::scoped_lock lock(*internalLock);
	mprintf("Killing previously scheduled timer expiration");
	
	//boost::mutex::scoped_lock lock2(*schedule_node_lock);
	if(schedule_node != NULL){
		schedule_node->cancel();	// kill that one
	}
	
	*has_expired = true;
}

void Timer::startUS(MonkeyWorksTime howlongus){
	boost::mutex::scoped_lock lock(*internalLock);
	
	//boost::mutex::scoped_lock lock2(*schedule_node_lock);
	
	//	cerr << "Starting timer (" << howlongus << ")" << endl;
	
	if(schedule_node != NULL){
		schedule_node->cancel();	// kill that one
	}
	
	if(howlongus <= 0){
		mwarning(M_SYSTEM_MESSAGE_DOMAIN,
				 "Scheduling a timer to fire at a time in the past");
		*has_expired = true;
		return;
	}
	
	
	*has_expired = false;
	
	shared_ptr<Scheduler> scheduler = Scheduler::instance();
	shared_ptr<Timer> this_one = shared_from_this();
	schedule_node = scheduler->scheduleUS(FILELINE,
										  howlongus,
										  0, 
										  1,
										  boost::bind(expireTheTimer, this_one),
										  M_DEFAULT_PRIORITY,
										  M_DEFAULT_WARN_SLOP_US,
										  M_DEFAULT_FAIL_SLOP_US,
										  M_MISSED_EXECUTION_CATCH_UP); 
}

void Timer::setExpired(bool has_it){
	boost::mutex::scoped_lock lock(*internalLock);
	*has_expired = has_it;
}


bool Timer::hasExpired(){
	boost::mutex::scoped_lock lock(*internalLock);
	return *has_expired;
}

void Timer::cleanUp(){}

Data Timer::getValue() {
	boost::mutex::scoped_lock lock(*internalLock);
	return Data(M_BOOLEAN, *has_expired);
}

////////////////////////////////////////////////////////////////////////////
// A polymorphic copy constructor (inherited from Clonable)
////////////////////////////////////////////////////////////////////////////
Variable *Timer::clone(){
	Timer *returned = 
	new Timer((const Timer&)(*this));
	return (Variable *)returned;
}


namespace mw {
	void *expireTheTimer(const shared_ptr<Timer> &timer){
		timer->setExpired(true);
		return NULL;
	}
}


