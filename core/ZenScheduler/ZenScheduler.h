/*
 *  ZenScheduler.h
 *  MWorksCore
 *
 *  Created by David Cox on 6/16/05.
 *  Copyright 2005 MIT. All rights reserved.
 *
 */

#ifndef ZEN_SCHEDULER_H
#define ZEN_SCHEDULER_H

#include "MWorksCore/Scheduler.h"
#include "MWorksCore/ExpandableList.h"
#include "MWorksCore/LinkedList.h"
#include "MWorksCore/Plugin.h"
#include "boost/thread/mutex.hpp"
#include "boost/shared_ptr.hpp"

#include <map>
#include <string>


BEGIN_NAMESPACE_MW


#define SAFE_GET(type, A)   type result; lock(); result = A; unlock(); return result;
#define SAFE_SET(A, B)	   lock(); A = B; unlock();
//#define SAFE_GET(type, A) return A;
//#define SAFE_SET(A,B)  A = B;

//#undef REALTIME_SCHEDULER

#define BASE_NUM_TASKS 5
#define TASK_PRIORITY 64

#ifndef LOW_PRIORITY_MODE
#define ZEN_SCHEDULER_PLUGIN_NAME	"ZenScheduler"	
#else
#define ZEN_SCHEDULER_PLUGIN_NAME	"LowPriorityScheduler"
#endif


///////////////////////////////////////////////////////////////////////////////////////
// Plugin entry point
//
// The extern "C" bit ensures that the name of this function is not mangled (C++
// requires name-mangling in order to achieve function overriding.  Our plugin host
// needs to know what name to look for, and for this plugin architecture, that name
// is 'getPlugin'
///////////////////////////////////////////////////////////////////////////////////////

extern "C"{
    Plugin *getPlugin();
}


#ifdef LOW_PRIORITY_MODE
namespace low_priority_scheduler{
#else
	namespace zen_scheduler {
#endif
		
		void *zenScheduledExecutionThread(void *arglist);
		
		class ZenScheduler; // patience...
		
		
		class ZenScheduleTask : public ScheduleTask, public Lockable{
			
			// TODO: DDC: I think that the lock granularity here is too fine-grained.
			// Particularly for scheduled tasks that repeat on a millisecond-ish
			// timescale, there are several times more locks being acquired and
			// released than needed.
			// If it turns out that this represents a significant cost, we should
			// agglomerate all of the locked accesses together and have manual
			// lock/unlock calls from the outside
			
			//**********************
			// Scheduling parameters
			//**********************
			
			// is this who we think it is?
			long node_id;
			// who owns this task?
			shared_ptr<Scheduler> scheduler;
			// when was this task first scheduled
			MWTime start_time_us;
			// how long before first execution
			MWTime initial_delay_us;
			// how long between subsequent repeats
			MWTime repeat_interval_us;
			// how many times to repeat
			int ntimes;
			// what to do about missed execution
			MissedExecutionBehavior behavior;
			// priority
			int priority;
			// how much slop to warn about
			int warning_slop_us;
			// how much slop to fail over
			int fail_slop_us;
			// function
			boost::function<void *()> functor;
			// The thread that the task is running in
			pthread_t thread;
			
			//**********************
			// Task status
			//**********************
			
			int ndone;
			bool active;	
			bool alive;
			bool executing;
			
			
			// When was the last heartbeat?
			MWTime last_beat;
			
			// Real(er)time add-on.  Experimental
			MWTime computation_time_us;
			
			
			
		public:
			
			ZenScheduleTask(const std::string &description,
							 long id,
							 const shared_ptr<Scheduler> &_scheduler, 
							 boost::function<void *()> _functor,
							 MWTime _start_time,
							 MWTime _initial_delay, 
							 MWTime _repeat_interval, 
							 int _ntimes, 
							 int priority, 
							 MissedExecutionBehavior _behave,
							 MWTime _warn_slop, 
							 MWTime _fail_slop,
							 MWTime _computation_time_us);
			
			virtual ~ZenScheduleTask() {}
			
			
			virtual void cancel();
			virtual void pause();
			virtual void resume();
			virtual void kill();
			
			// Set a handle to the thread (in case you need to kill it)
			void setThread(pthread_t _thread){
				SAFE_SET(thread, _thread);
			}
			
			// heartbeat to let the scheduler know you're still alive
			virtual void heartbeat(){
				if(scheduler->getClock() == NULL){
					return;
				}
				
				SAFE_SET(last_beat, scheduler->getClock()->getCurrentTimeUS());
			}
			
			virtual MWTime getLastHeartbeat(){
				SAFE_GET(MWTime, last_beat);
			}
			
			bool isExecuting(){
				SAFE_GET(bool, executing);
			}
			
			void setExecuting(bool value){
				SAFE_SET(executing, value);
			}
			
			bool isAlive(){
				SAFE_GET(bool, alive);
			}
			
			void setAlive(bool value){
				SAFE_SET(alive, value);
			}
			
			bool isActive(){
				SAFE_GET(bool, active);
			}
			
			boost::function<void *()> getFunctor(){
				SAFE_GET(boost::function<void *()>, functor);
			}
			
			void setActive(bool value){
				SAFE_SET(active, value);
			}
						
			int getNDone(){
				SAFE_GET(int, ndone);
			}
			
			void setNDone(bool value){
				SAFE_SET(ndone, value);
			}
			
			
			MWTime getStartTimeUS(){
				SAFE_GET(MWTime, start_time_us);
			}
			
			MWTime getInitialDelayUS(){
				SAFE_GET(MWTime, initial_delay_us);
			}
			
			MWTime getRepeatIntervalUS(){
				SAFE_GET(MWTime, repeat_interval_us);
			}
			
			int getNTimes(){
				SAFE_GET(int, ntimes);
			}
			
			// what to do about missed execution
			MissedExecutionBehavior getMissedExecutionBehavior(){
				SAFE_GET(MissedExecutionBehavior, behavior);
			}
			
			int getPriority(){
				SAFE_GET(int, priority);
			}
			
			MWTime getWarningSlopUS(){
				SAFE_GET(MWTime, warning_slop_us);
			}
			
			MWTime getFailureSlopUS(){
				SAFE_GET(MWTime, fail_slop_us);
			}
			
			MWTime getComputationTimeUS(){
				SAFE_GET(MWTime, computation_time_us);
			}
			
			
			//		ZenScheduler *getScheduler(){
			//			SAFE_GET(ZenScheduler *, scheduler);
			//		}
			
			long getNodeID(){
				SAFE_GET(long, node_id);
			}
			
			std::string getDescription() {
				SAFE_GET(std::string, description);
			}
			
			shared_ptr<Scheduler> getScheduler() {
				lock();
				shared_ptr<Scheduler> return_scheduler = scheduler;
				unlock();
				
				return return_scheduler;
			}
		};
		
		
		class ZenScheduler : public Scheduler, public ComponentFactory  {
			
		protected:
			
			// Each scheduled task gets an id; this maps between the
			// ids and the tasks themselves.
			std::map<long, shared_ptr<ZenScheduleTask> > all_tasks;
			
			int nscheduled;
			
			boost::mutex scheduler_lock;
			
		public:
			
			ZenScheduler(const shared_ptr<Clock> &a_clock);
			~ZenScheduler();
			
			// factory method
			virtual shared_ptr<mw::Component> createObject(
														std::map<std::string, std::string> parameters,
														ComponentRegistry *reg){
				shared_ptr <Clock> a_clock = Clock::instance();
				return shared_ptr<mw::Component>(new ZenScheduler(a_clock));
			}
			
			
			virtual shared_ptr<ScheduleTask> scheduleUS(const std::string &description,
														 MWTime initial_delay, 
														 MWTime repeat_interval, 
														 int ntimes, 
														 boost::function<void *()> _functor,
														 int _priority,
														 MWTime _warn_slop_us,
														 MWTime _fail_slop_us,
														 MissedExecutionBehavior behav);
			
			
			// Remove a task from the task list
			virtual void removeTask(long task_id){
				//boost::mutex::scoped_lock lock(scheduler_lock, true);
				//all_tasks.erase(task_id);
			}
			
			// Check to make sure none of the tasks are spinning unduely
			virtual void checkTasks();
			
			virtual void launchWatchdogThread();
		};
		
		
		class ZenSchedulerPlugin : public Plugin {
			
			virtual void registerComponents(shared_ptr<ComponentRegistry> registry) {
				//cerr << "Registering zen scheduler as: " << ZEN_SCHEDULER_PLUGIN_NAME << endl;
				registry->registerFactory(std::string(ZEN_SCHEDULER_PLUGIN_NAME),
										  (ComponentFactory *)(new ZenScheduler(shared_ptr<Clock>())));
			}
		};
		
	} // end namespace


END_NAMESPACE_MW
	

#endif