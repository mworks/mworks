/*
 *  ZenScheduler.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 6/16/05.
 *  Copyright 2005 MIT. All rights reserved.
 *
 */

#include "ZenScheduler.h"


#include <mach/mach_types.h>
#include <mach/mach_init.h>
#include <mach/thread_policy.h>
#include <mach/task_policy.h>
#include <mach/thread_act.h>
#include <sys/sysctl.h>

#include <iostream>

#include "MachUtilities.h"

#ifndef	LOW_PRIORITY_MODE
#define SILENCE_SCHEDULE_WARNINGS 0
#else
#define SILENCE_SCHEDULE_WARNINGS 1
#endif


BEGIN_NAMESPACE_MW


Plugin *getPlugin() {
	#ifdef LOW_PRIORITY_MODE
		return new low_priority_scheduler::ZenSchedulerPlugin();
	#else
		return new zen_scheduler::ZenSchedulerPlugin();
	#endif
}


#ifdef LOW_PRIORITY_MODE
	namespace low_priority_scheduler{
#else
	namespace zen_scheduler {
#endif


void *zenScheduledExecutionThread(void *arglist) {
	// Hand-off the task description
	shared_ptr<ZenScheduleTask> *task_ptr = (shared_ptr<ZenScheduleTask> *)arglist;
	shared_ptr<ZenScheduleTask> task = *task_ptr;
	delete task_ptr;	// moved until end for speediness
	
#if REALTIME_SCHEDULER
#ifndef	LOW_PRIORITY_MODE
    int didit = set_realtime(task->getPriority());
    if(didit){
        //fprintf(stderr, "Scheduler went realtime.  Rock on.\n");
        //fflush(stderr);
    } else {
        fprintf(stderr,"Scheduler didn't go realtime.  Bummer.\n");
        fflush(stderr);
    }
#endif
#endif
	
	while (!task->isCanceled()) {
        MWTime next_us = task->execute(*(task->getScheduler()->getClock()), !SILENCE_SCHEDULE_WARNINGS);
        if (next_us < 0) {
            break;
        }
        task->getScheduler()->getClock()->sleepUS(next_us - task->getScheduler()->getClock()->getCurrentTimeUS());
	}
	
	return 0;
	
}



ZenScheduleTask::ZenScheduleTask(const std::string &_description,
								   const shared_ptr<ZenScheduler> &_scheduler,
								   boost::function<void *()> _functor,
								   MWTime _start_time,
								   MWTime _initial_delay, 
								   MWTime _repeat_interval, 
								   int _ntimes, int  _priority,
								   MissedExecutionBehavior _behavior,
								   MWTime _warn_slop, 
								   MWTime _fail_slop) :
    ScheduleTask(_description,
                 _start_time,
                 _initial_delay,
                 _repeat_interval,
                 _ntimes,
                 _functor,
                 _priority,
                 _warn_slop,
                 _fail_slop,
                 _behavior),
    scheduler(_scheduler)
{ }


shared_ptr<ScheduleTask> ZenScheduler::scheduleUS(const std::string &description,
													MWTime initial_delay, 
													MWTime repeat_interval, 
													int ntimes,
													boost::function<void *()> _functor,
													int _priority,
													MWTime _warn_slop_us, 
													MWTime _fail_slop_us, 
													MissedExecutionBehavior _behav){
	
	MWTime start_time = the_clock->getCurrentTimeUS();
	
	shared_ptr<ZenScheduler> self_shared_ptr(component_shared_from_this<ZenScheduler>());
	
	// Build up a description of the task to schedule
	shared_ptr<ZenScheduleTask> *task_ptr = new shared_ptr<ZenScheduleTask>(new ZenScheduleTask(description, 
																								   self_shared_ptr,
																								   _functor,
																								   start_time,
																								   initial_delay, 
																								   repeat_interval, 
																								   ntimes, 
																								   _priority, 
																								   _behav,
																								   _warn_slop_us, 
																								   _fail_slop_us));
	
	// This pointer hand-off is necessary to get the task description passed
	// into the scheduled function (that function will call delete on the 
	// the extra copy of the shared_ptr)
//	shared_ptr<ZenScheduleTask> *task_ptr = 
//		new shared_ptr<ZenScheduleTask>();
//	*task_ptr = task;
	
	shared_ptr<ZenScheduleTask> task = *task_ptr;
	
    pthread_t thread;
	int status = pthread_create(&thread,
								NULL, 
								zenScheduledExecutionThread, 
								task_ptr);
	
	if(status != 0){
		switch(status){
			
			case EAGAIN:
				mwarning(M_SCHEDULER_MESSAGE_DOMAIN,
						 "System lacks resources necessary to create another scheduling thread");
				break;
			case EINVAL:
				mwarning(M_SCHEDULER_MESSAGE_DOMAIN,
						 "Invalid thread parameters during new thread creation");
				break;
			case EPERM:
				mwarning(M_SCHEDULER_MESSAGE_DOMAIN,
						 "Permissions error on new thread creation");
				break;
		}
		shared_ptr<ScheduleTask> empty;
		return 	empty;
	}
	
	pthread_detach(thread);
	
	struct sched_param sp;
    memset(&sp, 0, sizeof(struct sched_param));
    
#ifdef	LOW_PRIORITY_MODE
	sp.sched_priority = 0;
#else
	sp.sched_priority=_priority;
#endif
	
#ifdef	REALTIME_SCHEDULER
	status = pthread_setschedparam(thread, SCHED_FIFO, &sp);
#else
	status = pthread_setschedparam(thread, SCHED_RR, &sp);
#endif
	
	if(status != 0){
		
		switch(status){
			case EINVAL:
				mwarning(M_SCHEDULER_MESSAGE_DOMAIN,
						 "Failed to set thread priority: settings invalid");
				break;
				
			case EFAULT:
				mwarning(M_SCHEDULER_MESSAGE_DOMAIN,
						 "Failed to set thread priority: invalid param pointer");
				break;
				
			case ENOTSUP:
				mwarning(M_SCHEDULER_MESSAGE_DOMAIN,
						 "Failed to set thread priority: unsupported setting");
				break;
				
			case ESRCH:
				mwarning(M_SCHEDULER_MESSAGE_DOMAIN,
						 "Failed to set thread priority: thread not running");
				break;
		}
		
	}
	
	
	// Return a shared_ptr to the task so that the party that scheduled
	// this task can cancel it if needed.
	shared_ptr<ScheduleTask> returntask = boost::dynamic_pointer_cast<ScheduleTask, ZenScheduleTask>(task); 
	return returntask;	
}


} // end namespace


END_NAMESPACE_MW
