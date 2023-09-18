/*
 *  ZenStateSystem.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 6/16/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */

#include "ZenStateSystem.h"

#include <MWorksCore/Experiment.h>
#include <MWorksCore/MachUtilities.h>
#include <MWorksCore/Scheduler.h>
#include <MWorksCore/StandardVariables.h>
#include <MWorksCore/Utilities.h>


BEGIN_NAMESPACE_MW

    
StandardStateSystem::StandardStateSystem(const boost::shared_ptr<Clock> &clock) :
    StateSystem(clock),
    is_running(false),
    is_paused(false)
{ }


StandardStateSystem::~StandardStateSystem() {
    if (state_system_thread.joinable()) {
        state_system_thread.join();
    }
}


void StandardStateSystem::start() {
    lock_guard lock(state_system_mutex);
    
    if (is_running) {
        return;
    }
    
    mprintf(M_STATE_SYSTEM_MESSAGE_DOMAIN, "Called start on state system");
    
    // Clean up the previous thread, if any
    if (state_system_thread.joinable()) {
        state_system_thread.join();
    }
    
    state_system_thread = std::thread([this]() { run(); });
    
    is_running = true;
    sendSystemStateEvent();
}


void StandardStateSystem::stop() {
    lock_guard lock(state_system_mutex);
    
    if (!is_running) {
        return;
    }
    
    mprintf(M_STATE_SYSTEM_MESSAGE_DOMAIN, "Called stop on state system");
    (*state_system_mode) = STOPPING;
    
    sendSystemStateEvent();
}


void StandardStateSystem::pause() {
    lock_guard lock(state_system_mutex);
    
    if (!is_running || is_paused) {
        return;
    }
    
    mprintf(M_STATE_SYSTEM_MESSAGE_DOMAIN, "Pausing state system");
    is_paused = true;
    (*state_system_mode) = PAUSED;
    
    sendSystemStateEvent();
}


void StandardStateSystem::resume() {
    lock_guard lock(state_system_mutex);
    
    if (!is_running || !is_paused) {
        return;
    }
    
    mprintf(M_STATE_SYSTEM_MESSAGE_DOMAIN, "Resuming paused state system");
    is_paused = false;
    (*state_system_mode) = RUNNING;
    
    sendSystemStateEvent();
}


boost::shared_ptr<State> StandardStateSystem::getCurrentState() {
    // Allow access to the current state only on the state system thread
    if (std::this_thread::get_id() == state_system_thread.get_id()) {
        return current_state.lock();
    }
    return boost::shared_ptr<State>();
}


void StandardStateSystem::run() {
    if (!(MachThreadSelf("MWorks State System").setPriority(TaskPriority::Default))) {
        merror(M_SCHEDULER_MESSAGE_DOMAIN, "Failed to set priority of state system thread");
    }
    
    // Make a copy of the experiment to ensure that it isn't destroyed before we're done with it
    auto current_experiment = GlobalCurrentExperiment;
    if (!current_experiment) {
        merror(M_STATE_SYSTEM_MESSAGE_DOMAIN, "Cannot start state system without a valid experiment defined");
        return;
    }
    
    current_state = current_experiment;
    auto current_state_shared = current_state.lock();
    
    mprintf(M_STATE_SYSTEM_MESSAGE_DOMAIN, "Starting state system....");
    (*state_system_mode) = RUNNING;
    
    boost::weak_ptr<State> next_state;
    boost::shared_ptr<State> next_state_shared;
    bool in_action = false;
    bool in_transition = false;
    
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
        
        if (!in_transition) {
            in_action = true;
            
            try {
                current_state_shared->action();
            } catch (const std::exception &e) {
                merror(M_PARADIGM_MESSAGE_DOMAIN, "Stopping state system: %s", e.what());
                (*state_system_mode) = STOPPING;
                break;
            }
            
            // finished performing action
            in_action = false;
        }
        
        if (!in_action) {
            in_transition = true;
            
            try {
                next_state = current_state_shared->next();
            } catch (const std::exception &e) {
                merror(M_PARADIGM_MESSAGE_DOMAIN, "Stopping state system: %s", e.what());
                (*state_system_mode) = STOPPING;
                break;
            }
            
            next_state_shared = next_state.lock();
            if (!next_state_shared) {
                // no next state yet, sleep until the next tick
                continue;
            }
            
            current_state = next_state;
            current_state_shared = next_state_shared;
            
            next_state.reset();
            next_state_shared.reset();
            
            // finished transition
            in_transition = false;
        }
    }
    
    {
        lock_guard lock(state_system_mutex);
        
        is_running = false;
        is_paused = false;
        
        (*state_system_mode) = IDLE;
        mprintf(M_STATE_SYSTEM_MESSAGE_DOMAIN, "State system ending");
        
        mprintf(M_STATE_SYSTEM_MESSAGE_DOMAIN, "Resetting experiment");
        current_experiment->reset();
    }
}


END_NAMESPACE_MW
