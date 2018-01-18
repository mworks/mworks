/*
 *  ZenStateSystem.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 6/16/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */


#include "MWorksCore/Experiment.h"
#include "ZenStateSystem.h"
#include "StateSystem.h"
#include "MWorksCore/Utilities.h"
#include "MWorksCore/Scheduler.h"
#include "MWorksCore/StateSystem.h"
#include "MWorksCore/StandardVariables.h"

#include "MachUtilities.h"


BEGIN_NAMESPACE_MW

    
StandardStateSystem::StandardStateSystem(const shared_ptr <Clock> &a_clock) :
    StateSystem(a_clock)
{
    in_action = false;
    in_transition = false;
    is_running = false;
    is_paused = false;
}


StandardStateSystem::~StandardStateSystem() {
    if (state_system_thread.joinable()) {
        state_system_thread.join();
    }
}


void StandardStateSystem::start(){
    boost::mutex::scoped_lock lock(state_system_mutex);
    
    if (is_running) {
        return;
    }
    
    //E->setInt(taskMode_edit, RUNNING);

//	(*state_system_mode) = RUNNING;
  
	mprintf("Called start on state system");
    
    // Make a copy of the experiment to ensure that it isn't destroyed before we're done with it
    shared_ptr<Experiment> current_experiment = GlobalCurrentExperiment;
	if (!current_experiment) {
		merror(M_STATE_SYSTEM_MESSAGE_DOMAIN,
			  "Cannot start state system without a valid experiment defined");
		return;
	}
    
    // Clean up the previous thread, if any
    if (state_system_thread.joinable()) {
        state_system_thread.join();
    }
	
	weak_ptr<State> exp_ref(current_experiment);
	current_experiment->setCurrentState(exp_ref);
    
    auto sharedThis = component_shared_from_this<StandardStateSystem>();
    state_system_thread = std::thread([sharedThis]() { sharedThis->run(); });
    
    is_running = true;
	sendSystemStateEvent();
	
}
    
    
void StandardStateSystem::stop(){
    boost::mutex::scoped_lock lock(state_system_mutex);
    
    if (!is_running) {
        return;
    }
    
    mprintf("Called stop on state system");
    (*state_system_mode) = STOPPING;
	
	// TODO: need to stop ongoing schedules...
	// esp. IO devices
    sendSystemStateEvent();
}

void StandardStateSystem::pause(){
    boost::mutex::scoped_lock lock(state_system_mutex);
    
    if (!is_running || is_paused) {
        return;
    }
    
    mprintf("Pausing state system");
    is_paused = true;
    (*state_system_mode) = PAUSED;
    
    sendSystemStateEvent();
}

void StandardStateSystem::resume(){
    boost::mutex::scoped_lock lock(state_system_mutex);
    
    if (!is_running || !is_paused) {
        return;
    }
    
    mprintf("Resuming paused state system");
    is_paused = false;
    (*state_system_mode) = RUNNING;
    
    sendSystemStateEvent();
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

//void StandardStateSystem::setInAction(bool isit){
//    in_action = isit;
//}

//void StandardStateSystem::setInTransition(bool isit){
//    in_transition = isit;
//}

weak_ptr<State> StandardStateSystem::getCurrentState(){
    // Allow access to the current state only on the state system thread
    if (std::this_thread::get_id() == state_system_thread.get_id()) {
        return current_state;
    }
    return weak_ptr<State>();
}


void StandardStateSystem::run() {
    if (!(MachThreadSelf("MWorks State System").setPriority(TaskPriority::Default))) {
        merror(M_SCHEDULER_MESSAGE_DOMAIN, "Failed to set priority of state system thread");
    }

    // Make a copy of the experiment to ensure that it isn't destroyed before we're done with it
    shared_ptr<Experiment> current_experiment = GlobalCurrentExperiment;
    if (!current_experiment) {
        merror(M_STATE_SYSTEM_MESSAGE_DOMAIN,
               "Cannot start state system without a valid experiment defined");
        return;
    }

	mprintf("Starting state system....");


    //mprintf("----------setting task  mode to running------------");
	(*state_system_mode) = (long) RUNNING;
	current_state = current_experiment->getCurrentState();
	
	if(current_state.expired()){
		merror(M_STATE_SYSTEM_MESSAGE_DOMAIN,
				"current state is NULL. Shutting down state system...");
				
		(*state_system_mode) = (long)IDLE;
	}
		
	if(current_state.expired() == true){
		// TODO: better throw
		throw SimpleException("Invalid current state within state system");
	}
	
	shared_ptr<State> current_state_shared(current_state);
    weak_ptr<State> next_state;
	
	while (current_state_shared) {
        const bool canInterrupt = current_state_shared->isInterruptible();  // might not be an okay place to stop
        
        if (canInterrupt &&
            (long(*state_system_mode) == IDLE ||      // hard stop
             long(*state_system_mode) == STOPPING))   // stop requested
        {
            break;
        }
        else if (in_transition ||              // waiting for the next state
                 (canInterrupt && is_paused))  // paused
        {
            getClock()->yield();
            if (canInterrupt && is_paused) {
                continue;
            }
        }
		
		//mprintf("State system main loop, current state = %d", current_state);
		if (!in_transition) {
			in_action = true;
			
			//mState *test_state = current_state;
			
			try {
				current_state_shared->action();
			} catch(std::exception& e){
				merror(M_PARADIGM_MESSAGE_DOMAIN,
					   "Stopping state system: %s", e.what());
				state_system_mode->setValue((long)STOPPING);
				break;
			}
		
			// finished performing action
			in_action = false;
		}
		
		if (!in_action) {
			in_transition = true;

		
			try {
				next_state = current_state_shared->next();
			} catch (std::exception& e){
				merror(M_PARADIGM_MESSAGE_DOMAIN,
					  "Stopping state system: %s", e.what());
				state_system_mode->setValue((long)STOPPING);
				break;
			}
            
            if (next_state.expired()) {
                // no next state yet, sleep until the next tick
                continue;
            }
			
            shared_ptr<State> next_state_shared;
            
            try{
                shared_ptr<State> attempt(next_state); // cast weak_ptr into shared_ptr
                next_state_shared = attempt; // machination required because of weak to shared conversion semantics
			} catch (std::exception& e){
                mwarning(M_STATE_SYSTEM_MESSAGE_DOMAIN, "Failed to acquire shared_ptr from next_state; coming to an abrupt halt");
                (*state_system_mode) = IDLE;
                continue;
            }
			
			//mprintf("State system moving on... %d", next_state);
			
			current_experiment->setCurrentState(next_state);
			
			
			
			// If we've finished
			if(current_state_shared.get() == current_experiment.get() &&
					next_state_shared.get() == current_experiment.get()){
					mprintf("Returned to Experiment node, halting state system...");
					(*state_system_mode) = IDLE;
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
	
	
    {
        boost::mutex::scoped_lock lock(state_system_mutex);
        
        in_action = false;
        in_transition = false;
        is_running = false;
        is_paused = false;
        
        (*state_system_mode) = IDLE;    
        mprintf("State system ending");
        
        // DDC: graceful stop?
        mprintf("Resetting experiment");
        current_experiment->reset();
    }
}


END_NAMESPACE_MW



