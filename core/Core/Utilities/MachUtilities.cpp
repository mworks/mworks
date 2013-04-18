/*
 *  MachUtilities.cpp
 *  MWorksCore
 *
 *  Created by Christopher Stawarz on 5/13/11.
 *  Copyright 2011 MIT. All rights reserved.
 *
 */

#include "MachUtilities.h"

#include <mach/mach.h>

#include <boost/noncopyable.hpp>

#include "Utilities.h"


BEGIN_NAMESPACE_MW


//
// NOTE: Although Apple's docs don't mention it, it's crucial to release the port returned by
// mach_thread_self with mach_port_deallocate.  Otherwise, the port is leaked.  See
// http://technosloth.blogspot.com/2010/08/unexpected-mach-port-refcounts.html
//


class MachThreadSelf : boost::noncopyable {
    
public:
    MachThreadSelf() {
        port = mach_thread_self();
    }
    
    ~MachThreadSelf() {
        if (KERN_SUCCESS != mach_port_deallocate(mach_task_self(), port)) {
            mwarning(M_SCHEDULER_MESSAGE_DOMAIN,
                     "Unable to release Mach thread self port; application is leaking system resources");
        }
    }
    
    thread_port_t get() const {
        return port;
    }
    
private:
    thread_port_t port;
    
};


int set_realtime(int period, int computation, int constraint) {
    MachThreadSelf machThread;

    thread_time_constraint_policy_data_t ttcpolicy;
    ttcpolicy.period = period;
    ttcpolicy.computation = computation;
    ttcpolicy.constraint = constraint;
    ttcpolicy.preemptible = 1;

    kern_return_t ret = thread_policy_set(machThread.get(),
                                          THREAD_TIME_CONSTRAINT_POLICY,
                                          (thread_policy_t)(&ttcpolicy),
                                          THREAD_TIME_CONSTRAINT_POLICY_COUNT);
    
    if (KERN_SUCCESS != ret)  {
        merror(M_SCHEDULER_MESSAGE_DOMAIN,
               "Set realtime failed (error code: %d, period = %d, computation = %d, constraint = %d)",
               ret, period, computation, constraint);
        return 0;
    }
    
    return 1;
}


int set_realtime(int priority) {
    MachThreadSelf machThread;
    
    kern_return_t result = KERN_SUCCESS;

    integer_t timeShareData;
    if (priority > 64) {
        timeShareData = 0;
    } else {
        timeShareData = 1;
    }
    
    integer_t precedenceData = priority;
    
    // Set the scheduling flavor. We want to do this first, since doing so
    // can alter the priority
    result = thread_policy_set(machThread.get(),
                               THREAD_EXTENDED_POLICY,
                               &timeShareData,
                               THREAD_EXTENDED_POLICY_COUNT);
    
    if (KERN_SUCCESS != result)
        return 0;
    
    // Now set the priority
    result = thread_policy_set(machThread.get(),
                               THREAD_PRECEDENCE_POLICY,
                               &precedenceData,
                               THREAD_PRECEDENCE_POLICY_COUNT);
    
    if (KERN_SUCCESS != result)
        return 0;
    
    return 1;
}


END_NAMESPACE_MW






















