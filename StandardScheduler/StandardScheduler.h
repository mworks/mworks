#ifndef STANDARD_SCHEDULER_H
#define STANDARD_SCHEDULER_H

#include "MonkeyWorksCore/Scheduler.h"
#include "MonkeyWorksCore/ExpandableList.h"
#include "MonkeyWorksCore/LinkedList.h"
#include "MonkeyWorksCore/RealtimeServices.h"
#include "MonkeyWorksCore/Plugin.h"


#define REALTIME_SCHEDULER

#include <string.h>
#define BASE_NUM_TASKS 5    
#define TASK_PRIORITY 0


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


void *scheduledExecutionThread(void *arglist);

class ScheduleTask : public ScheduleTask, public LinkedListNode, public Lockable {

	long initial_delay, repeat_interval, starttime, lasttime;
	int ntimes, ndone;
	pthread_cond_t condition;
	pthread_mutex_t mutex;
	
	
	// a mutex and condition to ensure that the task thread is properly primed
    //pthread_cond_t primed_condition;
	//pthread_mutex_t primed_mutex;
        
	pthread_t thread;
	bool active, alive;
	
	
	int n_pending;
	bool executing; // a measure to avoid spins
	

	public:
	
        ScheduleTask();
        
        ScheduleTask(long _initial_delay);
        ScheduleTask(long _repeat_interval, int _ntimes);
		ScheduleTask(long _initial_delay, long _repeat_interval, int _ntimes);
		void tick(long currenttime);
            
		void signal();
		void activate();
	
		bool isActive();
        bool *getAlivePointer();
	
	
		void incrementPendingSignal(){
			lock();
			n_pending++;
			unlock();
		}
		
		void decrementPendingSignal(){
			lock();
			n_pending--;
			unlock();
		}
		
		bool hasPendingSignal(){
			lock();
			bool n = n_pending;
			unlock();
		
			if(n){
				return true;
			} else {
				return false;
			}
		}
		
			
		bool isExecuting(){
			bool returnval;
			lock();
			returnval = executing;
			unlock(); 
			return executing;
		}
		bool setExecuting(bool value){
			lock();
			executing = value;
			unlock();
            return true;
		}
	
		pthread_cond_t *getCondition();
		pthread_mutex_t *getMutex();
	
        pthread_t *getThread();
	
        //pthread_cond_t *getPrimedCondition();
        //pthread_mutex_t *getPrimedMutex();
        
        void setPriority(int _priority);
        void setPriority();
        void kill();
        

        
        void cancel();
        void pause();
        void resume();
};


typedef struct THREAD_ARGUMENT_LIST {
	ScheduleTask *task;
	void *(*function)(void *);
	void *argument;
} ThreadArgumentList;


class StandardScheduler : public Scheduler, public SchedulerFactory{

	LinkedList *schedulelist;
	//mExpandableList<ScheduleTask> *schedulelist;
	//pthread_cond_t direct_tick_condition;
	
	public:
	
	StandardScheduler();

        // factory method
	Scheduler *createScheduler();        
        
	virtual ScheduleTask *schedule(long initial_delay, long repeat_interval, int ntimes, void *(*funptr)(void *), void *arg);
	virtual ScheduleTask *scheduleMS(long initial_delay, long repeat_interval, int ntimes, void *(*funptr)(void *), void *arg);
	virtual ScheduleTask *scheduleUS(long initial_delay, long repeat_interval, int ntimes, void *(*funptr)(void *), void *arg);
        
	virtual void tick();
	virtual void tick(long time);
	
	virtual pthread_cond_t *getDirectTickCondition();
	
};


class StandardSchedulerPlugin : public Plugin {

   
    virtual ExpandableList<ServiceDescription> *getServiceDescriptions(){
    
        ExpandableList<ServiceDescription> *list = new ExpandableList<ServiceDescription>();
        
        // only one service in this plugin (just the scheduler), but a plugin could, in principle,
        // contain a whole suite of related services.
        
        list->addElement(new ServiceDescription("StandardScheduler", (ObjectFactory *)(new StandardScheduler()),M_SCHEDULER_COMPONENT));
    
        return list;
    }
};


#endif