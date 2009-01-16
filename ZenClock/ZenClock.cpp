/**
 * ZenClock.cpp
 *
 * History:
 * David Cox on Sat Dec 28 2002 - Created.
 *
 * Copyright (c) 2002 MIT. All rights reserved.
 */

#define HZ 800000

#include <Carbon/Carbon.h>

#include <CoreServices/CoreServices.h>

#include "ZenClock.h"

#include "MonkeyWorksCore/RealtimeServices.h"

#include <mach/mach_types.h>
#include <mach/mach_init.h>
#include <mach/thread_policy.h>
#include <mach/task_policy.h>
#include <mach/thread_act.h>
//#include <mach/mach_time.h>
//#include <mach/time.h>
//#include <mach/sched.h>

#include <sys/sysctl.h>

//#include "kern/clock.h"

//#include <IOKit/IOLib.h>
//#include <IOKit/IOPlatformExpert.h>

//#include <clock.h>

int set_realtime(int period, int computation, int constraint);

TMTask clockTMInfo = {0}; // a Time Manager information record
TimerUPP NativeISRUPP;
long clock_interval;
pthread_t clock_thread;
bool fullrealtime;

static long currentClockTime;
static long lastSystemTime;

static long getCarbonSystemTimeMS();
static long getCarbonSystemTimeUS();
static long getCarbonSystemTimeNS();
static void * clockISR(void *);

/********************************************************
 *          Non-Static function definitions
 *******************************************************/
Plugin *getPlugin() {
    return new StandardClockPlugin();
}

int set_realtime(int period, int computation, int constraint) {
    struct thread_time_constraint_policy ttcpolicy;
    int ret;

    ttcpolicy.period=period; // HZ/160
    ttcpolicy.computation=computation; // HZ/3300;
    ttcpolicy.constraint=constraint; // HZ/2200;
    ttcpolicy.preemptible=0;

    if ((ret=thread_policy_set(mach_thread_self(), 
                            THREAD_TIME_CONSTRAINT_POLICY,
                             (int *)&ttcpolicy, 
                             THREAD_TIME_CONSTRAINT_POLICY_COUNT))
                                                        != KERN_SUCCESS) {
        mprintf("set_realtime failed");
        return 0;
    }
    return 1;
}

/********************************************************
 *              Static function definitions
 *******************************************************/
static long getCarbonSystemTimeNS() {
//    long thetime;
    AbsoluteTime uptime;
    Nanoseconds nano;
    //uint64_t uptime;
    //uint64_t nano;
    uptime = UpTime();
    nano = AbsoluteToNanoseconds(uptime);
    //uptime = mach_absolute_time();
    //absolutetime_to_nanoseconds(uptime, &nano);
    
    return (long)(UnsignedWideToUInt64( nano ));
}

static long getCarbonSystemTimeUS() {
//    long thetime;
    AbsoluteTime uptime;
    Nanoseconds nano;
    //uint64_t uptime;
   // uint64_t nano;
    uptime = UpTime();
    nano = AbsoluteToNanoseconds(uptime);
    //uptime = mach_absolute_time();
    //absolutetime_to_nanoseconds(uptime, &nano);
    
    return (long)(UnsignedWideToUInt64( nano ) / 1000);
}
static long getCarbonSystemTimeMS() {
//    long thetime;
    AbsoluteTime uptime;
    Nanoseconds nano;
    //uint64_t uptime;
   // uint64_t nano;
    uptime = UpTime();
    nano = AbsoluteToNanoseconds(uptime);
    //uptime = mach_absolute_time();
    //absolutetime_to_nanoseconds(uptime, &nano);
    
    return (long)(UnsignedWideToUInt64( nano ) / 1000000);
}

static pascal void * clockISR(void * arg) {
    int bus_speed, mib [2] = { CTL_HW, HW_BUS_FREQ };
    size_t len;   
    len = sizeof (bus_speed);
    sysctl (mib, 2, &bus_speed, &len, NULL, 0);
    if(!fullrealtime) {
        // set real time
        set_realtime(bus_speed/1000, bus_speed/100000, bus_speed/2000);
        fullrealtime = true;
    }
//    static MonkeyWorksTime counter = 1;
    while(1) {
      //  long currentSystemTime = getCarbonSystemTimeUS();
     //   long deltaMS = (currentSystemTime/1000 - lastSystemTime/1000);
     //   if(deltaMS) {
    //        currentClockTime += deltaMS;
     //       lastSystemTime = currentSystemTime;
        GlobalScheduler->tick();  // set up deferred checkschedule
     //       if(deltaMS > 1){
               // mprintf("correct clock drift of %d ms", deltaMS - 1);
      //      }
     //   }
        struct timespec time_to_sleep;
        time_to_sleep.tv_nsec = 1000 * 1000; // check every 1000 usec
        nanosleep(&time_to_sleep, NULL);
        //nanosleep(0, 1000000);
    }
}

/***************************************************************
 *                  StandardClock Methods
 **************************************************************/
StandardClock::StandardClock(long interval_microseconds) 
                        : Clock(interval_microseconds), ClockFactory() {
    interval = -1000; // this version will just ignore the argument for now.
    clock_interval = interval;
    currentClockTime = 0;
    
}

long StandardClock::getSystemTime() {
    return getCarbonSystemTimeMS();
}
    
long StandardClock::getCurrentTimeMS() {
    return getCarbonSystemTimeMS();
}

long StandardClock::getCurrentTimeUS() {
    return getCarbonSystemTimeUS();
}

long StandardClock::getCurrentTimeNS() {
    return getCarbonSystemTimeNS();
}

long StandardClock::getInterval() {
    return interval;
}

void StandardClock::setInterval(long microseconds) {
    interval = microseconds;
}

void StandardClock::startClock() {
    pthread_create(&clock_thread, NULL, clockISR, NULL);
    
    struct sched_param sp;
    memset(&sp, 0, sizeof(struct sched_param));
    sp.sched_priority=0;
    
    pthread_setschedparam(clock_thread, SCHED_FIFO, &sp);
        
    fullrealtime = false;
    lastSystemTime = getCarbonSystemTimeUS();
}

void StandardClock::stopClock() {
//    kill(&clock_thread, 0);
    pthread_cancel(clock_thread);
}

Clock *StandardClock::createClock() {
    return (Clock *)(new StandardClock(1000));
}

/***************************************************************
 *                  StandardClockPlugin Methods
 **************************************************************/
ExpandableList<ServiceDescription> * 
                    StandardClockPlugin::getServiceDescriptions() {
    
    ExpandableList<ServiceDescription> *list = 
                            new ExpandableList<ServiceDescription>();
        
    // only one service in this plugin (just the scheduler),
    // but a plugin could, in principle,
    // contain a whole suite of related services.
    list->addElement(new ServiceDescription("StandardClock",
                        (ObjectFactory *)(new StandardClock(1000)), 
                                                        M_CLOCK_COMPONENT));
    return list;
}
