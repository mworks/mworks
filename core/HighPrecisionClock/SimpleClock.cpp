//
//  SimpleClock.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 8/17/16.
//
//

#include "SimpleClock.hpp"


BEGIN_NAMESPACE_MW


SimpleClock::SimpleClock() :
    systemBaseTimeAbsolute(mach_absolute_time())
{ }


MWTime SimpleClock::getSystemBaseTimeNS() {
    return timebase.absoluteToNanos(systemBaseTimeAbsolute);
}


MWTime SimpleClock::getSystemTimeNS() {
    return timebase.absoluteToNanos(mach_absolute_time());
}


MWTime SimpleClock::getCurrentTimeNS() {
    return timebase.absoluteToNanos(mach_absolute_time() - systemBaseTimeAbsolute);
}


void SimpleClock::sleepNS(MWTime time) {
    logMachError("mach_wait_until", mach_wait_until(mach_absolute_time() +
                                                    timebase.nanosToAbsolute(std::max(MWTime(0), time))));
}


void SimpleClock::yield() {
    sleepUS(500);
}


END_NAMESPACE_MW

























