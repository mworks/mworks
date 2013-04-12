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


BEGIN_NAMESPACE_MW


/********************************************************
 *          Non-Static function definitions
 *******************************************************/
Plugin *getPlugin() {
    return new StandardClockPlugin();
}



/***************************************************************
 *                  StandardClock Methods
 **************************************************************/
StandardClock::StandardClock() {
    mach_timebase_info(&tTBI);
	baseTime = mach_absolute_time();
}

MWTime StandardClock::getSystemTimeNS() {
    uint64_t nano = mach_absolute_time();
	return (MWTime)(nano * tTBI.numer / tTBI.denom);
}
    
MWTime StandardClock::getSystemBaseTimeNS() {
    return (MWTime)(baseTime * tTBI.numer / tTBI.denom);
}
    
MWTime StandardClock::getCurrentTimeNS() {
    uint64_t nano = mach_absolute_time() - baseTime;
    return (MWTime)(nano * tTBI.numer / tTBI.denom);
}

void StandardClock::sleepNS(MWTime time){ 
	uint64_t now = mach_absolute_time();
	mach_wait_until(now + (uint64_t)time * tTBI.denom / tTBI.numer);
}
    

END_NAMESPACE_MW


























