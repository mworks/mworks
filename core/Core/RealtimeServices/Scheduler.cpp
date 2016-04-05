/*
 *  Scheduler.cpp
 *  Experimental Control with Cocoa UI
 *
 *  Created by David Cox on Thu Jan 02 2003.
 *  Copyright (c) 2003 __MyCompanyName__. All rights reserved.
 *
 */


#include "Scheduler.h"


BEGIN_NAMESPACE_MW


MWTime ScheduleTask::execute(Clock &clock, bool doWarnings) {
    MWTime now_us = clock.getCurrentTimeUS();
    
    while (next_us > 0 && next_us <= now_us) {
        functor();
        
        if (doWarnings) {
            const MWTime slop_us = now_us - next_us;
            if (fail_slop_us > 0 && slop_us > fail_slop_us) {
                merror(M_SCHEDULER_MESSAGE_DOMAIN,
                       "Scheduled task (%s) not on time (off by %lld; task = %p; priority= %d)",
                       description.c_str(),
                       slop_us,
                       this,
                       priority);
            } else if (warning_slop_us > 0 && slop_us > warning_slop_us) {
                mwarning(M_SCHEDULER_MESSAGE_DOMAIN,
                         "Scheduled task (%s) not on time (off by %lld; task = %p; priority=%d)",
                         description.c_str(),
                         slop_us,
                         this,
                         priority);
            }
        }
        
        ndone++;
        
        if ((ntimes != M_REPEAT_INDEFINITELY && ndone >= ntimes) ||
            isCanceled())
        {
            next_us = -1;
            break;
        }
        
        next_us += repeat_interval_us;
        now_us = clock.getCurrentTimeUS();
        
        if ((now_us - next_us) > (repeat_interval_us)) {
            //
            // We're already late for the next execution.  Respond appropriately.
            //
            switch (behavior) {
                case M_MISSED_EXECUTION_FAIL:
                    merror(M_SCHEDULER_MESSAGE_DOMAIN,
                           "Scheduled task (%s) failed to keep up; terminating",
                           description.c_str());
                    next_us = -1;
                    break;
                    
                case M_MISSED_EXECUTION_DROP:
                    if (repeat_interval_us) {
                        const long ndropped = (now_us - next_us) / repeat_interval_us;
                        ndone += ndropped;
                        next_us += ndropped * repeat_interval_us;
                        if (doWarnings && warning_slop_us > 0) {
                            mwarning(M_SCHEDULER_MESSAGE_DOMAIN,
                                     "Scheduled task (%s) falling behind, dropping %ld scheduled executions "
                                     "(priority = %d, interval = %lld, task = %p)",
                                     description.c_str(),
                                     ndropped,
                                     priority,
                                     repeat_interval_us,
                                     this);
                        }
                    }
                    break;
                    
                default:
                    // Try to catch up
                    break;
            }
        }
    }
    
    return next_us;
}


SINGLETON_INSTANCE_STATIC_DECLARATION(Scheduler)


END_NAMESPACE_MW


























