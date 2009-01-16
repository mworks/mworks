/*
 *  Scheduler.cpp
 *  Experimental Control with Cocoa UI
 *
 *  Created by David Cox on Thu Jan 02 2003.
 *  Copyright (c) 2003 __MyCompanyName__. All rights reserved.
 *
 */


#include "Scheduler.h"
#include "Utilities.h"
using namespace mw;

template <> shared_ptr<Scheduler> RegisteredSingleton<Scheduler>::singleton_instance = shared_ptr<Scheduler>();

Scheduler::Scheduler(const boost::shared_ptr <Clock> &a_clock) {
	the_clock = a_clock;
};


shared_ptr<ScheduleTask> Scheduler::scheduleMS(const std::string &description,
												 MonkeyWorksTime initial_delay_ms, 
												 MonkeyWorksTime repeat_interval_ms, 
												 int ntimes, 
												 boost::function<void *()> _functor,
												 int priority, 
												 MonkeyWorksTime warn_slop, 
												 MonkeyWorksTime fail_slop,
												 MissedExecutionBehavior behav) {
	return scheduleUS(description,
					  initial_delay_ms*1000,
					  repeat_interval_ms*1000,
					  ntimes,
					  _functor,
					  priority,
					  warn_slop*1000,
					  fail_slop*1000,
					  behav);
}

void Scheduler::fork(boost::function<void *()> _functor,
					  int priority){
	scheduleUS(FILELINE,0,0,1, _functor, priority, -1, -1, M_MISSED_EXECUTION_CATCH_UP);
}

boost::shared_ptr<Clock> Scheduler::getClock() const {
	return the_clock;
}	
