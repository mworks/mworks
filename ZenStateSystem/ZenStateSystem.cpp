/*
 *  ZenStateSystem.cpp
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 6/16/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */

#include "ZenStateSystem.h"

/*
 *  StandardStateSystem.cpp
 *  Experimental Control with Cocoa UI
 *
 *  Created by David Cox on Mon Dec 30 2002.
 *  Copyright (c) 2002 __MyCompanyName__. All rights reserved.
 *
 */

#include "StateSystem.h"

#include "MonkeyWorksCore/Utilities.h"
#include "MonkeyWorksCore/Scheduler.h"
#include "MonkeyWorksCore/StateSystem.h"
#include "MonkeyWorksCore/StandardVariables.h"
#include "MonkeyWorksCore/Experiment.h"
#include "MonkeyWorksCore/IODeviceManager.h"
#include "MonkeyWorksCore/Debugging.h"


#include <mach/mach_types.h>
#include <mach/mach_init.h>
#include <mach/thread_policy.h>
#include <mach/task_policy.h>
#include <mach/thread_act.h>
#include <sys/sysctl.h>

#ifdef	LOW_PRIORITY_MODE
	#define STATE_SYSTEM_PRIORITY	0
#else
	#define STATE_SYSTEM_PRIORITY 96
#endif

pthread_mutex_t state_system_mutex;
pthread_t state_system_thread;

int set_realtime(int priority);
int set_realtime(int period, int computation, int constraint);

static bool in_action, in_transition, is_running, is_paused;
static weak_ptr<State> current_state;


void *checkStateSystem(void *noargs);

    
StandardStateSystem::StandardStateSystem(const shared_ptr <Clock> &a_clock) : StateSystem(a_clock) {
    is_running = false;
    is_paused = false;
}

void StandardStateSystem::start(){

    //E->setInt(taskMode_edit, RUNNING);

//	(*task_mode) = RUNNING;
  
	mprintf("Called start on state system");
	
	if(GlobalCurrentExperiment == NULL){
		merror(M_STATE_SYSTEM_MESSAGE_DOMAIN,
			  "Cannot start state system without a valid experiment defined");
		return;
	}
	
	// TODO: is this implicit cast kosher?
	weak_ptr<State> exp_ref(GlobalCurrentExperiment);
	GlobalCurrentExperiment->setCurrentState(exp_ref);
	//GlobalCurrentExperiment->setCurrentState(GlobalCurrentExperiment->getCurrentProtocol());
		
	shared_ptr <StateSystem> *shared_ptr_to_this_ptr = new shared_ptr<StateSystem>(this->shared_from_this());
	pthread_create(&state_system_thread, NULL, checkStateSystem, shared_ptr_to_this_ptr);
    
    struct sched_param sp;
    memset(&sp, 0, sizeof(struct sched_param));
    sp.sched_priority=STATE_SYSTEM_PRIORITY;
    
    pthread_setschedparam(state_system_thread, SCHED_RR, &sp);
    
    is_running = true;
	sendSystemStateEvent();
	
}
    
    
void StandardStateSystem::stop(){
    // stop this thing somehow....

	if(task_mode != NULL){
		// is_running = false;
		(*task_mode) = IDLE;
	}
	
	
	// TODO: need to stop ongoing schedules...
	// esp. IO devices
    sendSystemStateEvent();
}

void StandardStateSystem::pause(){
    // send in a message to suspend the state system
//    sendSystemPausedEvent();
    //is_paused = true;
}

bool StandardStateSystem::isRunning(){
    return is_running;
}

bool StandardStateSystem::isPaused(){
    return is_paused;
}

bool StandardStateSystem::isInAction(){
    return in_action;
}

bool StandardStateSystem::isInTransition(){
    return in_transition;
}

void StandardStateSystem::setInAction(bool isit){
    in_action = isit;
}

void StandardStateSystem::setInTransition(bool isit){
    in_transition = isit;
}

weak_ptr<State> StandardStateSystem::getCurrentState(){
    return current_state;
}

void StandardStateSystem::setCurrentState(weak_ptr<State> newcurrent){
    current_state = newcurrent;
}

//  check state system
void *checkStateSystem(void *void_state_system){

	// Hand-off the self state system reference
	shared_ptr<StateSystem> *ss_ptr = (shared_ptr<StateSystem> *)void_state_system;
	shared_ptr<StateSystem> ss = *ss_ptr;
	delete ss_ptr;

	
	weak_ptr<State> next_state;


	int bus_speed, mib [2] = { CTL_HW, HW_BUS_FREQ };
	size_t len;   
	len = sizeof (bus_speed);
	sysctl (mib, 2, &bus_speed, &len, NULL, 0);
	
	int didit = set_realtime(STATE_SYSTEM_PRIORITY);
	
	if(didit){
		//fprintf(stderr,"Scheduler went realtime.  Rock on.\n");
		//fflush(stderr);
	} else {
		fprintf(stderr,"Scheduler didn't go realtime.  Bummer.\n");
		fflush(stderr);
	}
	
   // set_realtime(bus_speed/1000, bus_speed/4000, bus_speed/2000);


	mprintf("Starting state system....");


	if(GlobalCurrentExperiment == NULL){
		merror(M_STATE_SYSTEM_MESSAGE_DOMAIN,
				"GlobalCurrentExperiment is not defined.");
		return NULL;
	}
    //mprintf("----------setting task  mode to running------------");
	(*task_mode) = (long) RUNNING;
	current_state = GlobalCurrentExperiment->getCurrentState();
    
	//mExperiment *testexp = GlobalCurrentExperiment;
	
	if(current_state.expired()){
		merror(M_STATE_SYSTEM_MESSAGE_DOMAIN,
				"current state is NULL. Shutting down state system...");
				
		(*task_mode) = (long)IDLE;
	}
        
        //while(1){
		
	if(current_state.expired() == true){
		// TODO: better throw
		throw SimpleException("Invalid current state within state system");
	}
	
	shared_ptr<State> current_state_shared(current_state);
	
	while((current_state_shared != NULL			  &&		// broken system
	      (long)(*task_mode) != IDLE      &&			// hard stop
		  (long)(*task_mode) != STOPPING)  ||		// stop requested
								!(current_state_shared->isInterruptible())){ // might not be
																			   // an okay place to stop
		
		// this is a global bool; it is set a by a listener to the variable
		// debuggerActive
		if(debugger_enabled){
			debuggerCheck();
		}
		
		
		
		//mprintf("State system main loop, current state = %d", current_state);
		if(!in_action){
			in_action = true;
			
			//mState *test_state = current_state;
			
			try {
				current_state_shared->action();
			} catch(std::exception& e){
				merror(M_PARADIGM_MESSAGE_DOMAIN,
					   "Stopping state system: %s", e.what());
				task_mode->setValue((long)STOPPING);
				break;
			}
		
			// finished performing action
			in_action = false;
		}
		
		if(!in_transition){
			in_transition = true;

		
			try {
				next_state = current_state_shared->next();
			} catch (std::exception& e){
				merror(M_PARADIGM_MESSAGE_DOMAIN,
					  "Stopping state system: %s", e.what());
				task_mode->setValue((long)STOPPING);
				break;
			}
			
			while(next_state.expired()){// && E->getInt(taskMode_edit) == RUNNING){
				
				if(current_state_shared->isInterruptible() &&
					((long)(*task_mode) == IDLE  ||			// hard stop
					 (long)(*task_mode) == STOPPING)){
					 next_state = GlobalCurrentExperiment;
					 break;
				}
				
				// no next state yet, sleep until the next tick
				ss->getClock()->sleepUS(500);
				try {
					next_state = current_state_shared->next();
				} catch (std::exception& e){
					merror(M_PARADIGM_MESSAGE_DOMAIN,
						  "Stopping state system: %s", e.what());
				}
				
			}
			
			shared_ptr<State> next_state_shared(next_state);
			
			
			//mprintf("State system moving on... %d", next_state);
			
			GlobalCurrentExperiment->setCurrentState(next_state);
			
			
			
			// If we've finished
			if(current_state_shared.get() == GlobalCurrentExperiment.get() && 
					next_state_shared.get() == GlobalCurrentExperiment.get()){
					mprintf("Returned to Experiment node, halting state system...");
					(*task_mode) = IDLE;
					current_state = weak_ptr<State>();
					next_state = weak_ptr<State>();
					continue;
			}
			
			
			current_state = next_state;
			current_state_shared = shared_ptr<State>(current_state);
			
			next_state = weak_ptr<State>();
			next_state_shared = shared_ptr<State>();
			
			// finished transition
			in_transition = false;
		}
	}
	
	
	in_action = false;
	in_transition = false;
	is_running = false;
    (*task_mode) = IDLE;    
	mprintf("State system ending");
	
	
	// DDC: graceful stop?
	if(GlobalCurrentExperiment != NULL){
		mprintf("Reseting experiment");
		GlobalCurrentExperiment->reset();
	}
	
	if(GlobalIODeviceManager != NULL){
		GlobalIODeviceManager->stopAllDeviceIO();
	}
	
	pthread_exit(0);
	return NULL;
}

int set_realtime(int priority){
		kern_return_t                       result = 0;
	    
		integer_t	timeShareData;
		integer_t	precedenceData;
		//thread_extended_policy_data_t       timeShareData;
	    //thread_precedence_policy_data_t     precedenceData;

	    //Set up some variables that we need for the task
	   
		precedenceData = priority;
		
		if(priority > 64){
			timeShareData = 0;
		} else {
			timeShareData = 1;
		}
	   // precedenceData.importance = priority;
	   // timeShareData.timeshare = true;//isTimeshare;
	    
	    mach_port_t  machThread = mach_thread_self();

	    //Set the scheduling flavor. We want to do this first, since doing so
	    //can alter the priority
	    result = thread_policy_set( machThread,
	                                THREAD_EXTENDED_POLICY,
	                                &timeShareData,
	                                THREAD_EXTENDED_POLICY_COUNT );

	    if( 0 != result )
	        return 0;
	
	    //Now set the priority
	    result =   thread_policy_set( machThread,
	                                THREAD_PRECEDENCE_POLICY,
	                                &precedenceData,
	                                THREAD_PRECEDENCE_POLICY_COUNT );

	    if( 0 != result )
	        return 0;

	return 1;
}


int set_realtime(int period, int computation, int constraint) {
    struct thread_time_constraint_policy ttcpolicy;
    int ret;

    ttcpolicy.period=period; // HZ/160
    ttcpolicy.computation=computation; // HZ/3300;
    ttcpolicy.constraint=constraint; // HZ/2200;
    ttcpolicy.preemptible=0;

    if ((ret=thread_policy_set(mach_thread_self(), THREAD_TIME_CONSTRAINT_POLICY, (int *)&ttcpolicy, THREAD_TIME_CONSTRAINT_POLICY_COUNT)) != KERN_SUCCESS) 
    {
            //mprintf("Set realtime failed (%d)", ret);
            return 0;
    }
    return 1;
}





