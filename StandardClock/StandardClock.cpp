/*
 *  Clock.cpp
 *  Experimental Control with Cocoa UI
 *
 *  Created by David Cox on Sat Dec 28 2002.
 *  Copyright (c) 2002 __MyCompanyName__. All rights reserved.
 *
 */

#include <Carbon/Carbon.h>
#include <CoreServices/CoreServices.h>

#include "StandardClock.h"
#include "MonkeyWorksCore/RealtimeServices.h"


static void  clockISR(TMTaskPtr tmTaskPtr);
static long getCarbonSystemTimeMS();
static long getCarbonSystemTimeUS();
static long getCarbonSystemTimeNS();

TMTask clockTMInfo = {0}; // a Time Manager information record
TimerUPP NativeISRUPP;
long clock_interval;
long requested_clock_interval;
static long currentClockTime;
static long lastSystemTime;

#define COUNT_TO_DO_DRIFT_CORRECT 100

static long drift_correction_accumulated;
static long drift_correction_count;

StandardClock::StandardClock(long interval_microseconds) : Clock(interval_microseconds), ClockFactory(){
    interval = -1000; // this version will just ignore the argument for now....
    requested_clock_interval = interval;
    clock_interval = interval;
    currentClockTime = 0;
    
    drift_correction_count = 0;
    
}
    
long StandardClock::getCurrentTimeMS(){
    return getCarbonSystemTimeMS();
}

long StandardClock::getCurrentTimeUS(){
    return getCarbonSystemTimeUS();
}
long StandardClock::getCurrentTimeNS(){
    return getCarbonSystemTimeNS();
}

long StandardClock::getInterval(){
    return interval;
}

void StandardClock::setInterval(long microseconds){
    interval = microseconds;
}

void StandardClock::startClock(){

    NativeISRUPP = NewTimerUPP((TimerProcPtr)clockISR);
    clockTMInfo.tmAddr = NativeISRUPP;						// Address of clock routine
    clockTMInfo.tmWakeUp = 0;								// set to NULL
    clockTMInfo.tmReserved = 0;								// set to NULL
    InstallXTimeTask((QElemPtr)&clockTMInfo);						// install record in Time Manager Queue
    PrimeTime((QElemPtr)&clockTMInfo, interval);		// activate the request

    lastSystemTime = getCarbonSystemTimeUS();
}

long StandardClock::getSystemTime(){
    return getCarbonSystemTimeMS();
}

void StandardClock::stopClock(){
    RmvTime((QElemPtr)&clockTMInfo);
    DisposeRoutineDescriptor(NativeISRUPP);
}



Clock *StandardClock::createClock(){
    return (Clock *)(new StandardClock(1000));
}


static long getCarbonSystemTimeMS(){
    return getCarbonSystemTimeUS() / 1000;
}

static long getCarbonSystemTimeNS(){
//    long thetime;
    AbsoluteTime uptime;
    Nanoseconds nano;
    //uint64_t uptime;
   // uint64_t nano;
    
    
    uptime = UpTime();
    nano = AbsoluteToNanoseconds(uptime);
    return (long)(UnsignedWideToUInt64( nano ) );

}

static long getCarbonSystemTimeUS(){
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

static pascal void clockISR(TMTaskPtr tmTaskPtr)
{
//    long actual_clock_interval;
    long currentSystemTime = getCarbonSystemTimeUS();

    long elapsed_time = (currentSystemTime - lastSystemTime) / 1000;

    if(elapsed_time){
        currentClockTime += elapsed_time;
        
/*
    drift_correction_accumulated += (currentSystemTime - lastSystemTime);
    drift_correction_count++;
    
    if(drift_correction_count > COUNT_TO_DO_DRIFT_CORRECT){
        actual_clock_interval == drift_correction_accumulated / drift_correction_count;
        if(actual_clock_interval != clock_interval){
           // clock_interval = (long)((float)requested_clock_interval * (float)requested_clock_interval / (float)actual_clock_interval);
            mprintf("drift acc: %d, drift count: %d", drift_correction_accumulated, drift_correction_count);
            mprintf("Clock drift corrected: %d", actual_clock_interval);
        }
        
        drift_correction_accumulated = 0;
        drift_correction_count = 0;
    }
*/
    
        GlobalScheduler->tick();  // set up deferred checkschedule
    }
    
    PrimeTime((QElemPtr)tmTaskPtr, clock_interval);
    
    lastSystemTime = currentSystemTime;
}


Plugin *getPlugin(){
    return new StandardClockPlugin();
}

