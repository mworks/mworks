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
    explicit ScheduleTask(const std::string &description) :
        description(description)
    { }
    
    virtual ~ScheduleTask() { }
    
    // Stop the task
    virtual void cancel() = 0;
    
    const std::string& getDescription() const {
        return description;
    }
    
private:
    std::string description;
    
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

