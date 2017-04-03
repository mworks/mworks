/*
 *  ZenScheduler.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 6/16/05.
 *  Copyright 2005 MIT. All rights reserved.
 *
 */

#include "ZenScheduler.h"

#include <thread>

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


static void zenScheduledExecutionThread(const boost::shared_ptr<ZenScheduleTask> &task) {
#ifndef	LOW_PRIORITY_MODE
    if (!(MachThreadSelf("MWorks Scheduled Task").setPriority(task->getPriority()))) {
        merror(M_SCHEDULER_MESSAGE_DOMAIN,
               "Failed to set priority of scheduled task (%s)",
               task->getDescription().c_str());
    }
#endif
	
	while (!task->isCanceled()) {
        MWTime next_us = task->execute(*(task->getScheduler()->getClock()), !SILENCE_SCHEDULE_WARNINGS);
        if (next_us < 0) {
            break;
        }
        task->getScheduler()->getClock()->sleepUS(next_us - task->getScheduler()->getClock()->getCurrentTimeUS());
	}
}


ZenScheduleTask::ZenScheduleTask(const std::string &_description,
                                 const shared_ptr<ZenScheduler> &_scheduler,
                                 boost::function<void *()> _functor,
                                 MWTime _start_time,
                                 MWTime _initial_delay,
                                 MWTime _repeat_interval,
                                 int _ntimes,
                                 TaskPriority _priority,
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
													TaskPriority _priority,
													MWTime _warn_slop_us, 
													MWTime _fail_slop_us, 
													MissedExecutionBehavior _behav)
{
	MWTime start_time = the_clock->getCurrentTimeUS();
	
	// Build up a description of the task to schedule
    auto task = boost::make_shared<ZenScheduleTask>(description,
                                                    component_shared_from_this<ZenScheduler>(),
                                                    _functor,
                                                    start_time,
                                                    initial_delay,
                                                    repeat_interval,
                                                    ntimes,
                                                    _priority,
                                                    _behav,
                                                    _warn_slop_us, 
                                                    _fail_slop_us);
    
    // Create and detach the task thread
    auto thread = std::thread([task]() { zenScheduledExecutionThread(task); });
    thread.detach();
	
	// Return a shared_ptr to the task so that the party that scheduled
	// this task can cancel it if needed.
	return task;
}


} // end namespace


END_NAMESPACE_MW



























