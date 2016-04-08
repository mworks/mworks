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

#include <atomic>
#include "Clock.h"
#include "MWorksTypes.h"
#include "Component.h"
#include "Utilities.h"
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>


BEGIN_NAMESPACE_MW


enum MissedExecutionBehavior {
    M_MISSED_EXECUTION_DROP,
    M_MISSED_EXECUTION_CATCH_UP,
    M_MISSED_EXECUTION_FAIL
};


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


class ScheduleTask {
    
public:
    virtual ~ScheduleTask() { }
    
    ScheduleTask(const std::string &description,
                 MWTime start_time_us,
                 MWTime initial_delay_us,
                 MWTime repeat_interval_us,
                 int ntimes,
                 boost::function<void *()> functor,
                 int priority,
                 MWTime warning_slop_us,
                 MWTime fail_slop_us,
                 MissedExecutionBehavior behavior) :
        description(description),
        repeat_interval_us(repeat_interval_us),
        ntimes(ntimes),
        functor(functor),
        priority(priority),
        warning_slop_us(warning_slop_us),
        fail_slop_us(fail_slop_us),
        behavior(behavior),
        next_us(start_time_us + initial_delay_us),
        ndone(0),
        canceled(false)
    { }
    
    bool isCanceled() const { return canceled.load(); }
    void cancel() { canceled.store(true); }
    
protected:
    MWTime execute(Clock &clock, bool doWarnings = true);
    
    const std::string description;
    const MWTime repeat_interval_us;
    const int ntimes;
    const boost::function<void *()> functor;
    const int priority;
    const MWTime warning_slop_us;
    const MWTime fail_slop_us;
    const MissedExecutionBehavior behavior;
    
private:
    MWTime next_us;
    long ndone;
    static_assert(ATOMIC_BOOL_LOCK_FREE == 2, "std::atomic_bool is not always lock-free");
    std::atomic_bool canceled;
    
};


class Scheduler {
    
public:
    virtual ~Scheduler() { }
    
    virtual shared_ptr<ScheduleTask> scheduleUS(const std::string &description,
                                                MWTime initial_delay_us,
                                                MWTime repeat_interval_us,
                                                int ntimes,
                                                boost::function<void *()> _functor,
                                                int priority,
                                                MWTime warn_slop,
                                                MWTime fail_slop,
                                                MissedExecutionBehavior behav) = 0;
    
    shared_ptr<ScheduleTask> scheduleMS(const std::string &description,
                                        MWTime initial_delay_ms,
                                        MWTime repeat_interval_ms,
                                        int ntimes,
                                        boost::function<void *()> _functor,
                                        int priority,
                                        MWTime warn_slop,
                                        MWTime fail_slop,
                                        MissedExecutionBehavior behav)
    {
        return scheduleUS(description,
                          initial_delay_ms * 1000,
                          repeat_interval_ms * 1000,
                          ntimes,
                          _functor,
                          priority,
                          warn_slop * 1000,
                          fail_slop * 1000,
                          behav);
    }
    
    REGISTERED_SINGLETON_CODE_INJECTION(Scheduler)
    
};


END_NAMESPACE_MW


#endif

