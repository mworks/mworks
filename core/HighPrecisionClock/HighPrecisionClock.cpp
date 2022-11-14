//
//  HighPrecisionClock.cpp
//  HighPrecisionClock
//
//  Created by Christopher Stawarz on 4/12/13.
//  Copyright (c) 2013 The MWorks Project. All rights reserved.
//

#include "HighPrecisionClock.h"


BEGIN_NAMESPACE_MW


HighPrecisionClock::HighPrecisionClock() :
    period(timebase.nanosToAbsolute(periodUS * nanosPerMicro)),
    computation(timebase.nanosToAbsolute(computationUS * nanosPerMicro)),
    running(false)
{ }


HighPrecisionClock::~HighPrecisionClock() {
    stopClock();
}


void HighPrecisionClock::startClock() {
    if (!isRunning()) {
        running = true;
        runLoopThread = std::thread([this] { runLoop(); });
    }
}


void HighPrecisionClock::stopClock() {
    if (isRunning()) {
        running = false;
        runLoopThread.join();
    }
}


void HighPrecisionClock::sleepNS(MWTime time) {
    wait(mach_absolute_time() + timebase.nanosToAbsolute(std::max(MWTime(0), time)));
}


void HighPrecisionClock::yield() {
    wait();
}


void HighPrecisionClock::wait(uint64_t expirationTime) {
    thread_local Semaphore sem;
    {
        lock_guard lock(waitsMutex);
        waits.emplace(expirationTime, sem);
    }
    sem.wait();
}


void HighPrecisionClock::runLoop() {
    if (!(MachThreadSelf("MWorks High Precision Clock").setRealtime(period, computation, period))) {
        merror(M_SCHEDULER_MESSAGE_DOMAIN, "HighPrecisionClock failed to achieve real time scheduling");
    }
    
    auto startTime = mach_absolute_time();
    
    while (running) {
        {
            lock_guard lock(waitsMutex);
            while (!waits.empty() && waits.top().getExpirationTime() <= startTime) {
                waits.top().getSemaphore().signal();
                waits.pop();
            }
        }
        
        // Sleep until the next work cycle
        startTime += period;
        if (mach_absolute_time() < startTime) {
            logMachError("mach_wait_until", mach_wait_until(startTime));
        }
    }
}


HighPrecisionClock::Semaphore::Semaphore() :
    semaphore(SEMAPHORE_NULL)
{
    throwMachError("semaphore_create", semaphore_create(mach_task_self(), &semaphore, SYNC_POLICY_FIFO, 0));
}


HighPrecisionClock::Semaphore::~Semaphore() {
    logMachError("semaphore_destroy", semaphore_destroy(mach_task_self(), semaphore));
}


void HighPrecisionClock::Semaphore::wait() {
    logMachError("semaphore_wait", semaphore_wait(semaphore));
}


void HighPrecisionClock::Semaphore::signal() {
    logMachError("semaphore_signal", semaphore_signal(semaphore));
}


END_NAMESPACE_MW
