/*
 *  ZenScheduler.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 6/16/05.
 *  Copyright 2005 MIT. All rights reserved.
 *
 */

#include "ZenScheduler.h"


#define VERBOSE_SCHEDULER 0

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

// JJD added	-- set to 0 for old behavior
#define CALM_THE_SCHEDULE_WARNINGS 1

#define ZEN_SCHEDULER_ROLLING_GARBAGE_NODES 1

#define CRAZY_SHIT_SCHEDULER	0


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

//#if (defined(__ppc__) && defined(__APPLE__))
int get_bus_speed()
{
    int mib[2];
    unsigned int miblen;
    int busspeed;
    int retval;
    size_t len;
	
    mib[0]=CTL_HW;
    mib[1]=HW_BUS_FREQ;
    miblen=2;
    len=4;
    retval = sysctl(mib, miblen, &busspeed, &len, NULL, 0);
    return busspeed;
}
//#endif


void *zenSchedulerWatchDog(void *arglist){
	
	//mprintf("Checking watch dog tasks");
	ZenScheduler *scheduler = (ZenScheduler *)arglist;
	
	scheduler->checkTasks();
	return 0;
}


void *zenScheduledExecutionThread(void *arglist){
	
	
	// Hand-off the task description
	shared_ptr<ZenScheduleTask> *task_ptr = (shared_ptr<ZenScheduleTask> *)arglist;
	shared_ptr<ZenScheduleTask> task = *task_ptr;
	delete task_ptr;	// moved until end for speediness
	
	//task->heartbeat();
	if(VERBOSE_SCHEDULER) mprintf("Scheduled thread spawned %p", task.get());
	
	
	// Unpack the task description
	boost::function<void *()> functor = task->getFunctor();
	int ntimes = task->getNTimes();
	MWTime initial_delay_us = task->getInitialDelayUS();
	MWTime repeat_interval_us = task->getRepeatIntervalUS();
	if(VERBOSE_SCHEDULER) mprintf(" ****** repeat_interval_us = %lu   %p", (unsigned long)repeat_interval_us, task.get());
	
	MWTime start_time_us = task->getStartTimeUS();
	MissedExecutionBehavior behavior = task->getMissedExecutionBehavior();
	int priority = task->getPriority();
	MWTime warning_slop_us = task->getWarningSlopUS();
	int fail_slop_us = task->getFailureSlopUS();
	MWTime computation_time_us = task->getComputationTimeUS();
	
	
	
	MWTime next_us;
	long ndone = 0;
	
	// Go realtime, if requested
	if(VERBOSE_SCHEDULER) mprintf("Computation time = %lld", computation_time_us);
	if(computation_time_us > 0LL){
		//#if (defined(__ppc__) && defined(__APPLE__))
		//mprintf("%d", get_bus_speed);
		
#ifndef	LOW_PRIORITY_MODE
		set_realtime(((double)get_bus_speed() * (double)repeat_interval_us) / 1000000.0, 
					 ((double)get_bus_speed() * (double)computation_time_us) / 1000000.0,  
					 ((double)get_bus_speed() * (double)repeat_interval_us) / 1000000.0);
#endif
		//#endif
	} else {
#if REALTIME_SCHEDULER
		
#ifndef	LOW_PRIORITY_MODE
		int didit = set_realtime(priority);
		if(didit){
			//fprintf(stderr, "Scheduler went realtime.  Rock on.\n");
			//fflush(stderr);
		} else {
			fprintf(stderr,"Scheduler didn't go realtime.  Bummer.\n");
			fflush(stderr);
		}
#endif
#endif
	}
	
	
	MWTime now_us = task->getScheduler()->getClock()->getCurrentTimeUS();
	
	//cerr << "Startup latency: " << now_us - start_time_us << endl;
	
	MWTime time_to_first_shot = (start_time_us + initial_delay_us) - now_us;
	// Sleep if there is an initial delay
	if(time_to_first_shot > 0){		
		task->getScheduler()->getClock()->sleepUS(time_to_first_shot);
	}
	
	
	// Compute the next scheduled execution time
	next_us = start_time_us + initial_delay_us + repeat_interval_us;
	
	
	
	if(VERBOSE_SCHEDULER) 
		mprintf("Scheduled thread entering main loop (%ld done, %d total)  %p",
				ndone, ntimes, task.get());
	
	
	while(task->isActive() && 
		  (ntimes == M_REPEAT_INDEFINITELY || ndone < ntimes)){
		
		
		//task->heartbeat();
		
		// *********************
		// Execute the payload
		// *********************
		
		if(VERBOSE_SCHEDULER){
			mprintf("Scheduled function called "
					"(main loop, %ld done / %d total) %p",
					ndone, ntimes, task.get());
		}
		
		now_us = task->getScheduler()->getClock()->getCurrentTimeUS();		
		functor();
		
		if(VERBOSE_SCHEDULER){ 
			mprintf("Scheduled function returned "
					"(main loop) %p", task.get());
		}
		
		
		
		if(fail_slop_us > (MWTime)0 &&
		   now_us - next_us > fail_slop_us){
			if(!SILENCE_SCHEDULE_WARNINGS){
				merror(M_SCHEDULER_MESSAGE_DOMAIN,
					   "Scheduled task (%s) not on time (off by %lld; task = %p; priority= %d)",
					   task->getDescription().c_str(),
					   now_us - next_us, 
					   task.get(), 
					   task->getPriority());
			}
			
		} else if(warning_slop_us > (MWTime)0 &&
				  now_us - next_us > warning_slop_us){
			if (!SILENCE_SCHEDULE_WARNINGS) {
				mwarning(M_SCHEDULER_MESSAGE_DOMAIN,
						 "Scheduled task (%s) not on time (off by %lld; task = %p; priority=%d)", 
						 task->getDescription().c_str(),
						 now_us - next_us, task.get(),
						 task->getPriority());
			}
		}
		
		
		// one closer to being done...
		ndone++;
		
		// communicate back with the task object
		task->setNDone(ndone);
		
		if(ntimes != M_REPEAT_INDEFINITELY && ndone >= ntimes){
			if(VERBOSE_SCHEDULER) 
				mprintf("Scheduled task has completed %ld executions, quitting  %p",
						ndone, task.get());
			break;
		}
		
		if(!task->isActive()){
			if(VERBOSE_SCHEDULER) 
				mprintf("Scheduled execution deactivated (2) %p", task.get());
			
			break;
		}
		
		// this is when the next one should happen
		next_us = start_time_us + initial_delay_us + (ndone)*repeat_interval_us;		
		
		now_us = task->getScheduler()->getClock()->getCurrentTimeUS();
		
		// are we going too slow?
		
		//	if (warned) mprintf("DROP decision values: now: %d   next: %d   now-next:  %d ms  repeat interval %d ms",
		//		(long)(now_us/1000), (long)(next_us/1000), (long)((now_us - next_us)/1000), (long)(repeat_interval_us/1000));
		
		// JJD change Jan 30, 2007.
		// if when the NEXT one should happen is already more than the repeat_interval behind, then we are already too late!
		// This is partly how we can get into the state with a bunch of warnings, but no correction by dropping.
		// If warnings are less than this, then it IS proper behavior to keep spitting out the warnings -- it is up to the person scheduling 
		// things to set the warning level appropriately (i.e. greater than the repeat interval plus some slop to deal with delays inside this routine).
		
		//if((now_us - next_us) > (repeat_interval_us + warning_slop_us)) {		// DDC old test 
		if((now_us - next_us) > (repeat_interval_us)) {							// JJD new test Jan 30, 2007
																				//if (now_us > next_us) {							// DDC Feb 12, 3007
			
			if(VERBOSE_SCHEDULER){
				mprintf("Thread not keeping up (off by %ld usec: now = %lu, next = %lu repeat = %lu; priority = %d)  %p",
						(long)(now_us - next_us),
						(unsigned long)now_us, 
						(unsigned long)next_us, 
						(unsigned long)repeat_interval_us, 
						priority,
						task.get());
			}
			
			
			switch(behavior){
				
				case M_MISSED_EXECUTION_DROP:
					if(repeat_interval_us){
						if(VERBOSE_SCHEDULER) 
							mprintf("Dropping (ndone was: %ld)  %p", ndone, task.get());
						ndone += 0 + 
							(int)((now_us - next_us) / repeat_interval_us);
						//mprintf("((now_us - next_us) / repeat_interval_us) = %lld", ((now_us - next_us) / repeat_interval_us));	
						// move on
						if(warning_slop_us > (MWTime)0){
							if (!SILENCE_SCHEDULE_WARNINGS){
								mwarning(M_SCHEDULER_MESSAGE_DOMAIN,
										 "Scheduled task (%s) falling behind, dropping %d "
										 "scheduled executions "
										 "(priority = %d, interval = %lld, task = %p)",
										 task->getDescription().c_str(),
										 1 + (int)((now_us - next_us) /
												   repeat_interval_us),
										 priority, 
										 repeat_interval_us,
										 task.get());
								
								//lastWarnTimeUS = now_us;	
								
								if(task.get() == NULL){
									merror(M_SCHEDULER_MESSAGE_DOMAIN,
										   "NULL task: something is very wrong");
								}
							}
						}
					}
					break; 
					
				case M_MISSED_EXECUTION_CATCH_UP:
					// go go go....
					continue;
					
				case M_MISSED_EXECUTION_FAIL:
					merror(M_SCHEDULER_MESSAGE_DOMAIN,
						   "Scheduled execution failed to keep up");
					// TODO: put on the brakes
					break;
			}
																				} 
		
		
		next_us = start_time_us + initial_delay_us + 
			((MWTime)(ndone))*repeat_interval_us;
		
		if((next_us - now_us) > 0){
			if(VERBOSE_SCHEDULER) 
				mprintf("Scheduled thread sleeping... (2: %lld) %p", 
						(next_us - now_us), task.get());
			task->getScheduler()->getClock()->sleepUS(next_us - now_us);
		}
	}
	
	
	task->getScheduler()->removeTask(task->getNodeID());
	
	
	if(VERBOSE_SCHEDULER) mprintf("Scheduled thread ending... %p", task.get());
	return 0;
	
}



ZenScheduleTask::ZenScheduleTask(const std::string &_description,
								   long _id,
								   const shared_ptr<Scheduler> &_scheduler, 
								   boost::function<void *()> _functor,
								   MWTime _start_time,
								   MWTime _initial_delay, 
								   MWTime _repeat_interval, 
								   int _ntimes, int  _priority,
								   MissedExecutionBehavior _behavior,
								   MWTime _warn_slop, 
								   MWTime _fail_slop,
								   MWTime _computation_time_us){
	
	description = _description;
	node_id = _id;	
	scheduler = _scheduler;
	functor = _functor;
	initial_delay_us = _initial_delay;
	repeat_interval_us = _repeat_interval;
	ntimes = _ntimes;
	priority = _priority;
	behavior = _behavior;
	warning_slop_us = _warn_slop;
	fail_slop_us = _fail_slop;
	active = 1;
	alive = 1;
	computation_time_us = _computation_time_us;	
	start_time_us = _start_time;
	
}



void ZenScheduleTask::kill(){
	lock();
	pthread_cancel(thread);
	//release();
	unlock();
}

/*void ZenScheduleTask::restart(){
lock();
pthread_cancel(thread);

ZenScheduler *scheduler = description->getScheduler();
MWTime repeat_interval_us = description->getRepeatIntervalUS();
MWTime warning_slop_us = description->getWarningSlopUS();
MWTime fail_slop_us = description->getFailureSlopUS();
long ntimes = description->getNTimes();
long ndone = description->getNDone();
long priority = description->getPriority();


scheduler->scheduleUS(0, repeat_interval_us, ntimes - ndone, 
					  description->getFunction(), 
					  description->getArgument(), 
					  priority, warning_slop_us, fail_slop_us, 
					  description->getMissedExecutionBehavior());

unlock();
}
*/


void ZenScheduleTask::cancel(){
	setActive(false);
}

void ZenScheduleTask::pause(){ /* do this later */}
void ZenScheduleTask::resume(){ /* do this later */}




		ZenScheduler::ZenScheduler(const shared_ptr<Clock> &a_clock) : Scheduler(a_clock) { 
	
	nscheduled = 0;
	
#ifdef LOW_PRIORITY_MODE
	//fprintf(stderr, "Running low priority scheduler\n");
	//fflush(stderr);
#endif
	
	//launchWatchdogThread();
}


void ZenScheduler::launchWatchdogThread(){
//	
//	// experimental
//	// launch a watchdog thread
//	scheduleUS(FILELINE, 
//			   1000*1000,
//			   100*1000, 
//			   M_REPEAT_INDEFINITELY, 
//			   zenSchedulerWatchDog, 
//			   (void *)this,
//			   0,
//			   -1,
//			   -1,
//			   M_MISSED_EXECUTION_DROP);
}

ZenScheduler::~ZenScheduler() {}

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
	
	boost::mutex::scoped_lock lock(scheduler_lock);
	nscheduled++;
	long node_id = nscheduled;
	
	pthread_t thread;
	
	shared_ptr<Scheduler> self_shared_ptr(component_shared_from_this<Scheduler>());
	
	// Build up a description of the task to schedule
	shared_ptr<ZenScheduleTask> *task_ptr = new shared_ptr<ZenScheduleTask>(new ZenScheduleTask(description, 
																								   node_id, 
																								   self_shared_ptr,
																								   _functor,
																								   start_time,
																								   initial_delay, 
																								   repeat_interval, 
																								   ntimes, 
																								   _priority, 
																								   _behav,
																								   _warn_slop_us, 
																								   _fail_slop_us,
																								   -1));
	
	// This pointer hand-off is necessary to get the task description passed
	// into the scheduled function (that function will call delete on the 
	// the extra copy of the shared_ptr)
//	shared_ptr<ZenScheduleTask> *task_ptr = 
//		new shared_ptr<ZenScheduleTask>();
//	*task_ptr = task;
	
	shared_ptr<ZenScheduleTask> task = *task_ptr;
	
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
	
	task->setThread(thread);
	
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
	
	
	// Keep track of the task here in the scheduler (so that we can cancel
	// it later if we need to stop everything).
	//all_tasks.push_back(task);
	
	if(VERBOSE_SCHEDULER) mprintf("%d tasks successfully scheduled", nscheduled);
	
	// Return a shared_ptr to the task so that the party that scheduled
	// this task can cancel it if needed.
	shared_ptr<ScheduleTask> returntask = boost::dynamic_pointer_cast<ScheduleTask, ZenScheduleTask>(task); 
	return returntask;	
}



/*shared_ptr<ScheduleTask> ZenScheduler::scheduleConstrainedUS(MWTime initial_delay, 
MWTime repeat_interval, int ntimes, void *(*funptr)(void *), 
void *arg,int nbytes,
MWTime _computation_time_us, 
MWTime _warn_slop_us, 
MWTime _fail_slop_us,
MissedExecutionBehavior _behav = M_MISSED_EXECUTION_DROP){
		  
	void *argument = (void *)malloc(nbytes);
	memcpy(argument, arg, nbytes);
	
	return scheduleConstrainedUS(initial_delay, repeat_interval, ntimes, 
								 funptr, argument, _computation_time_us, 
								 _warn_slop_us, _fail_slop_us, _behav);
}*/


/*shared_ptr<ScheduleTask> ZenScheduler::scheduleConstrainedUS(MWTime initial_delay, 
MWTime repeat_interval, int ntimes, void *(*funptr)(void *), 
void *arg, MWTime _computation_time_us, 
MWTime _warn_slop_us, 
MWTime _fail_slop_us,
MissedExecutionBehavior _behav = M_MISSED_EXECUTION_DROP){
	
	nscheduled++;
	
	shared_ptr<ZenScheduleTaskDescription> task_desc(
													  new ZenScheduleTaskDescription(
																					  this, funptr, arg, 
																					  initial_delay, repeat_interval, ntimes, 
																					  96, _behav,
																					  _warn_slop_us, _fail_slop_us,
																					  _computation_time_us));
	
	//all_tasks.addToFront(task);
	
	
	// TODO fix this!
	
	mprintf("%d tasks successfully scheduled", nscheduled);
	
	shared_ptr<ScheduleTask> empty;
	return empty;
	
}						*/



void ZenScheduler::checkTasks(){
	
	return;
	
	/*
	 
	 shared_ptr<ZenScheduleTask> current(all_tasks.getFrontmost());
	 
	 do{
		 
		 shared_ptr<ZenScheduleTaskDescription> description = current->getDescription();
		 
		 MWTime now = the_clock->getCurrentTimeUS();
		 MWTime last = description->getLastHeartbeat();
		 
		 // do something more elegant / configurable
		 
		 MWTime interval = description->getRepeatIntervalUS();
		 
		 if(interval >= 0){
			 continue;
		 }
		 
#define M_ZEN_SCHEDULER_WATCHDOG_WARNING_MULTIPLE	4
#define M_ZEN_SCHEDULER_WATCHDOG_ERROR_MULTIPLE		10
#define M_ZEN_SCHEDULER_WATCHDOG_CANCEL_MULTIPLE	20
		 MWTime lag = now - last;
		 if(lag > M_ZEN_SCHEDULER_WATCHDOG_CANCEL_MULTIPLE * interval){
			 merror(M_SCHEDULER_MESSAGE_DOMAIN,
					"RESTARTING TASK: Task %d failed to check in for %ul ms", lag);
			 current->restart();
			 continue;
		 }
		 
		 if(lag > M_ZEN_SCHEDULER_WATCHDOG_ERROR_MULTIPLE * interval){
			 merror(M_SCHEDULER_MESSAGE_DOMAIN,
					"Task %d failed to check in for %ul ms", lag);
			 continue;
		 }
		 
		 if(lag > M_ZEN_SCHEDULER_WATCHDOG_WARNING_MULTIPLE * interval){
			 mwarning(M_SCHEDULER_MESSAGE_DOMAIN,
					  "Task %d failed to check in for %ul ms", lag);
			 continue;
		 }
		 
	 } while(current = current->getNext());*/
	
}

} // end namespace


END_NAMESPACE_MW
