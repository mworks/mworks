#ifndef SCHEDULER_H
#define SCHEDULER_H

/*
 *  Scheduler.h
 *  Experimental Control with Cocoa UI
 *
 *  Created by David Cox on Fri Dec 27 2002.
 * Paul Jankunas on 1/24/06 - Adding virtual destructor.
 *  Copyright (c) 2002 __MyCompanyName__. All rights reserved.
 *
 */

#include "Clock.h"
#include "MonkeyWorksTypes.h"
#include "Component.h"
#include "Utilities.h"
#include "boost/shared_ptr.hpp"

#include "boost/enable_shared_from_this.hpp"
namespace mw {

enum MissedExecutionBehavior{ M_MISSED_EXECUTION_DROP,
	M_MISSED_EXECUTION_CATCH_UP,
M_MISSED_EXECUTION_FAIL};

#define M_REPEAT_INDEFINITELY -999
#define M_DEFAULT_PRIORITY  94


#ifndef	M_DEFAULT_WARN_SLOP_MS
#define M_DEFAULT_WARN_SLOP_MS	5LL
#endif

#ifndef M_DEFAULT_FAIL_SLOP_MS
#define M_DEFAULT_FAIL_SLOP_MS 10LL
#endif

#define M_DEFAULT_WARN_SLOP_US	M_DEFAULT_WARN_SLOP_MS*1000LL
#define M_DEFAULT_FAIL_SLOP_US  M_DEFAULT_FAIL_SLOP_MS*1000LL



#define M_DEFAULT_STIMULUS_PRIORITY  96

// jjd added
#define M_DEFAULT_IODEVICE_PRIORITY  95		
#define	M_DEFAULT_IODEVICE_WARN_SLOP_MS  M_DEFAULT_WARN_SLOP_MS
#define	M_DEFAULT_IODEVICE_FAIL_SLOP_MS  M_DEFAULT_FAIL_SLOP_MS
#define	M_DEFAULT_IODEVICE_WARN_SLOP_US  M_DEFAULT_WARN_SLOP_US
#define	M_DEFAULT_IODEVICE_FAIL_SLOP_US  M_DEFAULT_FAIL_SLOP_US

#define M_DEFAULT_NETWORK_PRIORITY  63
#define M_DEFAULT_NETWORK_WARN_SLOP_MS  0LL		// no warnings
#define M_DEFAULT_NETWORK_FAIL_SLOP_MS  3000LL  // 3s before we raise an alarm
// networks can be flakey
#define M_DEFAULT_NETWORK_WARN_SLOP_US  M_DEFAULT_NETWORK_WARN_SLOP_MS*1000LL
#define M_DEFAULT_NETWORK_FAIL_SLOP_US  M_DEFAULT_NETWORK_FAIL_SLOP_MS*1000LL

#include <pthread.h>

using namespace boost;

void *dummy_function(void *);



class ScheduleTask{
protected:
	std::string description;
public:
	
	ScheduleTask(){ };
	virtual ~ScheduleTask(){ };
	
	virtual bool isActive() = 0;
	virtual bool isAlive() = 0;
	
	// stop a task; this one is essential
	virtual void cancel() = 0;
	
	
	virtual void pause() = 0;
	virtual void resume() = 0;
	virtual void kill() = 0; // kill the thread; dangerous except in case 
	// of emergency
	virtual std::string getDescription() { 
		return description;
	}
};


class Scheduler : public mw::Component, public enable_shared_from_this<Scheduler>, public RegisteredSingleton<Scheduler> {
	
protected:
	pthread_cond_t direct_tick_condition;
	boost::shared_ptr <Clock> the_clock;
	
public:
	Scheduler(const boost::shared_ptr <Clock> &a_clock);
	virtual ~Scheduler() {}
	
	virtual shared_ptr<ScheduleTask> scheduleMS(const std::string &description,
												 MonkeyWorksTime initial_delay_ms, 
												 MonkeyWorksTime repeat_interval_ms, 
												 int ntimes, 
												 boost::function<void *()> _functor,
												 int priority, 
												 MonkeyWorksTime warn_slop, 
												 MonkeyWorksTime fail_slop,
												 MissedExecutionBehavior behav);
	
	virtual shared_ptr<ScheduleTask> scheduleUS(const std::string &description,
												 MonkeyWorksTime initial_delay_us, 
												 MonkeyWorksTime repeat_interval_us,
												 int ntimes, 
												 boost::function<void *()> _functor,
												 int priority, 
												 MonkeyWorksTime warn_slop, 
												 MonkeyWorksTime fail_slop,
												 MissedExecutionBehavior behav) = 0;
	
	
	
	virtual void removeTask(long id) = 0;
	
	virtual void fork(boost::function<void *()> _functor,
					  int priority);
	
	boost::shared_ptr<Clock> getClock() const;
	
	virtual void launchWatchdogThread(){   return;  }
};

}
#endif

