/**
 * Event.cpp
 *
 * History:
 * David Cox on Thu Dec 05 2002 - Created.
 * Paul Jankunas on 12/20/04 - Added NetworkEvent class.
 *
 * Copyright (c) 2002 MIT. All rights reserved.
 */

#include "Event.h"

#include "Clock.h"


BEGIN_NAMESPACE_MW


Event::Event(int code, const Datum &data) :
    Event(code, Clock::instance()->getCurrentTimeUS(), data)
{ }


boost::shared_ptr<Event> Event::getNextEvent(MWTime timeoutUS) const {
    std::unique_lock<std::mutex> lock(mutex);
    
    if (!nextEvent && (timeoutUS > 0)) {
        nextEventAvailable.wait_for(lock,
                                    std::chrono::microseconds(timeoutUS),
                                    [this]() { return bool(nextEvent); });
    }
    
    return nextEvent;
}


void Event::setNextEvent(const boost::shared_ptr<Event> &event) {
    lock_guard lock(mutex);
    nextEvent = event;
    if (nextEvent) {
        nextEventAvailable.notify_all();
    }
}


END_NAMESPACE_MW



























