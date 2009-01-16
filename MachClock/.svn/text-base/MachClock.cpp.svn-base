/**
 * MachClock.cpp
 *
 * History:
 * David Cox on Sat Dec 28 2002 - Created.
 * Paul Jankunas on 06/02/05 - Fixed compiler errors after migrating to new
 *          version of gcc (4.0).  fixed spacing
 *
 * Copyright (c) 2002 MIT. All rights reserved.
 */

//#define HZ 800000

//#include <Carbon/Carbon.h>

#include <CoreServices/CoreServices.h>

#include "MachClock.h"

#include <mach/mach_types.h>
#include <mach/mach_init.h>
#include <mach/thread_policy.h>
#include <mach/task_policy.h>
#include <mach/thread_act.h>
#include <errno.h>
#include <mach/mach_time.h>
//#include <mach/time.h>
//#include <mach/sched.h>

#include <sys/sysctl.h>


/********************************************************
 *          Non-Static function definitions
 *******************************************************/
Plugin *getPlugin() {
    return new StandardClockPlugin();
}



/***************************************************************
 *                  StandardClock Methods
 **************************************************************/
StandardClock::StandardClock(long interval_microseconds) 
                        : Clock(interval_microseconds), ComponentFactory() {
    interval = -1000; // this version will just ignore the argument for now
    //clock_interval = interval;
    ///currentClockTime = 0;
	
	//#ifdef USE_MACH_MOJO
	
    mach_timebase_info(&tTBI);
    //cv = ((long double) tTBI.numer) / ((long double) tTBI.denom);
	//fprintf(stderr, "cv = %llu", cv);fflush(stderr);
    //#endif
	baseTime = mach_absolute_time();
}

MonkeyWorksTime StandardClock::getSystemTimeMS() {
    uint64_t nano = mach_absolute_time();
	return (MonkeyWorksTime)(nano * tTBI.numer / tTBI.denom) / 1000000LL;
}

MonkeyWorksTime StandardClock::getSystemTimeUS() {
    uint64_t nano = mach_absolute_time();
	return (MonkeyWorksTime)(nano * tTBI.numer / tTBI.denom) / 1000LL;
}

MonkeyWorksTime StandardClock::getSystemTimeNS() {
    uint64_t nano = mach_absolute_time();
	return (MonkeyWorksTime)(nano * tTBI.numer / tTBI.denom);
}
    
MonkeyWorksTime StandardClock::getCurrentTimeMS() {
    uint64_t nano = mach_absolute_time() - baseTime;
	return (MonkeyWorksTime)(nano * tTBI.numer / tTBI.denom) / 1000000LL;
}

MonkeyWorksTime StandardClock::getCurrentTimeUS() {
	uint64_t nano = mach_absolute_time() - baseTime;
	return (MonkeyWorksTime)(nano *  tTBI.numer / tTBI.denom) / 1000LL;
}
MonkeyWorksTime StandardClock::getCurrentTimeNS() {
	// first things first
	/*uint64_t now = mach_absolute_time();
	return (MonkeyWorksTime)(now * tTBI.numer / tTBI.denom);*/
	
	AbsoluteTime uptime;
    Nanoseconds nano;

    uptime = UpTime();
    nano = AbsoluteToNanoseconds(uptime);
    
    return (MonkeyWorksTime)(UnsignedWideToUInt64( nano ) - baseTime);
}

void StandardClock::sleepMS(MonkeyWorksTime time){ 
	sleepNS((MonkeyWorksTime)time * (MonkeyWorksTime)1000000);
}
void StandardClock::sleepUS(MonkeyWorksTime time){ 
	//mprintf("sleepUS for %lld usec", time);
	sleepNS((MonkeyWorksTime)time * (MonkeyWorksTime)1000);
}

void StandardClock::sleepNS(MonkeyWorksTime time){ 
	

#ifdef USE_MACH_MOJO
	
	
	uint64_t now = mach_absolute_time();
	
	//mach_wait_until(now + (uint64_t)((long double)time / (long double)cv));
	mach_wait_until(now + (uint64_t)time * tTBI.denom / tTBI.numer);
	
#else
	
	long seconds = 0;
	long nano = 0;
	
	
	if((time - (MonkeyWorksTime)1000000000)){
		lldiv_t div = lldiv(time, (MonkeyWorksTime)1000000000);
		seconds = (long)(div.quot);
		nano = (long)(div.rem);
	} else {
//		mprintf("not bigger %lld", time - (MonkeyWorksTime)1000000000);
		nano = (long)time;
	}
	
//	mprintf("nanosleeping for %d seconds, %d ns", seconds, nano);
	
	struct timespec time_to_sleep;
	time_to_sleep.tv_sec = seconds;
	time_to_sleep.tv_nsec = nano; // check every 300 usec
	int result = nanosleep(&time_to_sleep, NULL);
	if(result < 0){
		char *resultstring = "Unknown error";
		switch(errno){
			case EINTR:
				resultstring = "Interrumpted by signal";
				break;
			case EINVAL:
				resultstring = "Invalid time value";
				break;
			case ENOSYS:
				resultstring = "Not supported";
				break;
			case EFAULT:
				resultstring = "Invalid address";
				break;
		}
		mprintf("Clock sleep error %d: %s (%d seconds, %d nano)", errno, resultstring, (long)seconds, (long)nano);
	}
	
#endif
}


long StandardClock::getInterval() {
    return interval;
}

void StandardClock::setInterval(long microseconds) {
    interval = microseconds;
}

void StandardClock::startClock() {
	// nothing to do
}

void StandardClock::stopClock() {
    // nothing to do
}
