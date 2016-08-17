//
//  SimpleClock.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 8/17/16.
//
//

#include "SimpleClock.hpp"


BEGIN_NAMESPACE_MW


mach_timebase_info_data_t SimpleClock::getTimebaseInfo() {
    mach_timebase_info_data_t timebaseInfo;
    if (logMachError("mach_timebase_info", mach_timebase_info(&timebaseInfo))) {
        throw SimpleException(M_SCHEDULER_MESSAGE_DOMAIN, "Unable to create HighPrecisionClock");
    }
    return timebaseInfo;
}


SimpleClock::SimpleClock() :
    timebaseInfo(getTimebaseInfo()),
    systemBaseTimeAbsolute(mach_absolute_time())
{ }


MWTime SimpleClock::getSystemBaseTimeNS() {
    return absoluteToNanos(systemBaseTimeAbsolute);
}


MWTime SimpleClock::getSystemTimeNS() {
    return absoluteToNanos(mach_absolute_time());
}


MWTime SimpleClock::getCurrentTimeNS() {
    return absoluteToNanos(mach_absolute_time() - systemBaseTimeAbsolute);
}


void SimpleClock::sleepNS(MWTime time) {
    logMachError("mach_wait_until", mach_wait_until(mach_absolute_time() + nanosToAbsolute(std::max(MWTime(0), time))));
}


void SimpleClock::yield() {
    sleepUS(500);
}


END_NAMESPACE_MW

























