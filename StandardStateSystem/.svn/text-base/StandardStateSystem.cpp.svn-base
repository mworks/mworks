/*
 *  StandardStateSystem.cpp
 *  Experimental Control with Cocoa UI
 *
 *  Created by David Cox on Mon Dec 30 2002.
 *  Copyright (c) 2002 __MyCompanyName__. All rights reserved.
 *
 */

#include "StandardStateSystem.h"

#include "MonkeyWorksCore/Utilities.h"
#include "MonkeyWorksCore/RealtimeServices.h"
#include "MonkeyWorksCore/Scheduler.h"
#include "MonkeyWorksCore/StateSystem.h"
#include "MonkeyWorksCore/StandardVariables.h"
#include "MonkeyWorksCore/Experiment.h"


#include <mach/mach_types.h>
#include <mach/mach_init.h>
#include <mach/thread_policy.h>
#include <mach/task_policy.h>
#include <mach/thread_act.h>
#include <sys/sysctl.h>



#define STATE_SYSTEM_PRIORITY 0;


pthread_mutex_t state_system_mutex;
pthread_t state_system_thread;


int set_realtime(int period, int computation, int constraint);

static bool in_action, in_transition, is_running, is_paused;
static State *current_state;


void *checkStateSystem(void *noargs);

    
StandardStateSystem::StandardStateSystem():StateSystemFactory(){
    is_running = false;
    is_paused = false;
}

void StandardStateSystem::start(){

    //E->setInt(taskMode_edit, RUNNING);

    pthread_create(&state_system_thread, NULL, checkStateSystem, NULL);
    
    struct sched_param sp;
    memset(&sp, 0, sizeof(struct sched_param));
    sp.sched_priority=STATE_SYSTEM_PRIORITY;
    
    pthread_setschedparam(state_system_thread, SCHED_RR, &sp);
    
    is_running = true;
    sendSystemStartedEvent();
}
    
    
void StandardStateSystem::stop(){
    // stop this thing somehow....

    // is_running = false;
    sendSystemStoppedEvent();
}

void StandardStateSystem::pause(){
    // send in a message to suspend the state system
    
    //is_paused = true;
    sendSystemPausedEvent();
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

State *StandardStateSystem::getCurrentState(){
    return current_state;
}

void StandardStateSystem::setCurrentState(State *newcurrent){
    current_state = newcurrent;
}

//  check state system
void *checkStateSystem(void *void_state_system){

        State *next_state;

        //mStandardStateSystem *state_system = (StandardStateSystem *)void_state_system;

        int bus_speed, mib [2] = { CTL_HW, HW_BUS_FREQ };
        size_t len;   
        len = sizeof (bus_speed);
        sysctl (mib, 2, &bus_speed, &len, NULL, 0);
        
        set_realtime(bus_speed/1000, bus_speed/4000, bus_speed/2000);


	pthread_cond_t *direct_tick = GlobalScheduler->getDirectTickCondition();


	if(in_action){
		// another thread is already executing an action
		pthread_exit(0);
		return NULL;
	}
	
	if(in_transition){
		// another thread is evaluating a transition
		pthread_exit(0);
		return NULL;
	}

	if(GlobalCurrentExperiment == NULL){
		merror(M_STATE_SYSTEM_MESSAGE_DOMAIN,
				"GlobalCurrentExperiment is not defined.");
		return NULL;
	}

	GlobalCurrentExperiment->set(task_mode, (long)RUNNING);
	current_state = GlobalCurrentExperiment->getCurrentState();
    
	//mExperiment *testexp = GlobalCurrentExperiment;
	
	if(current_state == NULL){
		merror(M_STATE_SYSTEM_MESSAGE_DOMAIN,
			"current state is NULL. Shutting down state system...");
		GlobalCurrentExperiment->set(task_mode, (long)IDLE);
	}
        
        //while(1){
	while(GlobalCurrentExperiment->getInt(task_mode) != IDLE && current_state != NULL){
	
		if(!in_action){
			in_action = true;
			
			//mState *test_state = current_state;
			
			current_state->action();
		
			// finished performing action
			in_action = false;
		}
		
		if(!in_transition){
			in_transition = true;
			
			while(!(next_state = current_state->next())){// && E->getInt(taskMode_edit) == RUNNING){
				// no next state yet, sleep until the next tick
				pthread_mutex_lock(&state_system_mutex);
				pthread_cond_wait(direct_tick, &state_system_mutex);
				pthread_mutex_unlock(&state_system_mutex);
			}
			
			GlobalCurrentExperiment->setCurrentState(next_state);
			current_state = next_state;
			next_state = NULL;
			
			// finished transition
			in_transition = false;
		}
	}
	
        is_running = false;
        
	pthread_exit(0);
	return NULL;
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
            mprintf("Set realtime failed (%d)", ret);
            return 0;
    }
    return 1;
}


Plugin *getPlugin(){
    return new StandardStateSystemPlugin();
}




