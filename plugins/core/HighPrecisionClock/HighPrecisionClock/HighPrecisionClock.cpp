//
//  HighPrecisionClock.cpp
//  HighPrecisionClock
//
//  Created by Christopher Stawarz on 4/12/13.
//  Copyright (c) 2013 The MWorks Project. All rights reserved.
//

#include "HighPrecisionClock.h"


BEGIN_NAMESPACE_MW


mach_timebase_info_data_t HighPrecisionClock::getTimebaseInfo() {
    mach_timebase_info_data_t timebaseInfo;
    if (logMachError("mach_timebase_info", mach_timebase_info(&timebaseInfo))) {
        throw SimpleException(M_SCHEDULER_MESSAGE_DOMAIN, "Unable to create HighPrecisionClock");
    }
    return timebaseInfo;
}


HighPrecisionClock::HighPrecisionClock() :
    timebaseInfo(getTimebaseInfo()),
    systemBaseTimeAbsolute(mach_absolute_time()),
    threadSpecificSemaphore(&destroySemaphore)
{ }


HighPrecisionClock::~HighPrecisionClock() {
    stopClock();
}


MWTime HighPrecisionClock::getSystemBaseTimeNS() {
    return absoluteToNanos(systemBaseTimeAbsolute);
}


MWTime HighPrecisionClock::getSystemTimeNS() {
    return absoluteToNanos(mach_absolute_time());
}


MWTime HighPrecisionClock::getCurrentTimeNS() {
    return absoluteToNanos(mach_absolute_time() - systemBaseTimeAbsolute);
}


void HighPrecisionClock::startClock() {
    if (!isRunning()) {
        try {
            runLoopThread = boost::thread(boost::bind(&HighPrecisionClock::runLoop,
                                                      component_shared_from_this<HighPrecisionClock>()));
        } catch (const boost::thread_resource_error &e) {
            throw SimpleException(M_SCHEDULER_MESSAGE_DOMAIN, "Unable to start HighPrecisionClock", e.what());
        }
    }
}


void HighPrecisionClock::stopClock() {
    if (isRunning()) {
        runLoopThread.interrupt();
        try {
            runLoopThread.join();
        } catch (const boost::system::system_error &e) {
            merror(M_SCHEDULER_MESSAGE_DOMAIN, "Unable to stop HighPrecisionClock: %s", e.what());
        }
    }
}


void HighPrecisionClock::sleepNS(MWTime time) {
    // Don't try to sleep for less than one period
    time = std::max(time, periodUS * nanosPerMicro);
    
    const uint64_t expirationTime = mach_absolute_time() + nanosToAbsolute(time);
    
    semaphore_t *sem = threadSpecificSemaphore.get();
    if (!sem) {
        semaphore_t newSem;
        if (logMachError("semaphore_create", semaphore_create(mach_task_self(), &newSem, SYNC_POLICY_FIFO, 0))) {
            // If we can't create the semaphore, do a low-precision wait with mach_wait_until, and hope
            // that semaphore_create will work next time
            logMachError("mach_wait_until", mach_wait_until(expirationTime));
            return;
        }
        threadSpecificSemaphore.reset(new semaphore_t(newSem));
        sem = threadSpecificSemaphore.get();
    }
    
    {
        lock_guard lock(waitsMutex);
        waits.push(WaitInfo(expirationTime, *sem));
    }
    
    logMachError("semaphore_wait", semaphore_wait(*sem));
}


void HighPrecisionClock::destroySemaphore(semaphore_t *sem) {
    if (sem) {
        logMachError("semaphore_destroy", semaphore_destroy(mach_task_self(), *sem));
        delete sem;
    }
}


void HighPrecisionClock::runLoop() {
    const uint64_t period = nanosToAbsolute(periodUS * nanosPerMicro);
    const uint64_t computation = nanosToAbsolute(computationUS * nanosPerMicro);
    
    if (!set_realtime(period, computation, period)) {
        merror(M_SCHEDULER_MESSAGE_DOMAIN, "HighPrecisionClock failed to achieve real time scheduling");
    }
    
    uint64_t startTime = mach_absolute_time();
    
    while (true) {
        {
            lock_guard lock(waitsMutex);
            while (!waits.empty() && waits.top().getExpirationTime() <= startTime) {
                logMachError("semaphore_signal", semaphore_signal(waits.top().getSemaphore()));
                waits.pop();
            }
        }
        
        // Give another thread a chance to terminate this one
        boost::this_thread::interruption_point();
        
        // Sleep until the next work cycle
        startTime += period;
        if (mach_absolute_time() < startTime) {
            logMachError("mach_wait_until", mach_wait_until(startTime));
        }
    }
}


END_NAMESPACE_MW

























