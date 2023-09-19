/*
 *  ZenStateSystem.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 6/16/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */

#include "ZenStateSystem.h"

#include <boost/scope_exit.hpp>

#include <MWorksCore/Experiment.h>
#include <MWorksCore/MachUtilities.h>
#include <MWorksCore/Scheduler.h>
#include <MWorksCore/StandardVariables.h>
#include <MWorksCore/Utilities.h>


BEGIN_NAMESPACE_MW

    
StandardStateSystem::StandardStateSystem(const boost::shared_ptr<Clock> &clock) :
    StateSystem(clock),
    endState(State::getEndState()),
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


bool StandardStateSystem::runState(const boost::shared_ptr<State> &state) {
    return runState(state, false);  // Only the main state system thread should pause
}


boost::weak_ptr<State> & StandardStateSystem::getCurrentStateWeakRef() {
    thread_local boost::weak_ptr<State> currentState;
    return currentState;
}


void StandardStateSystem::failWithException(const std::exception &e, const char *fileline) {
    merror(M_PARADIGM_MESSAGE_DOMAIN, "Execution ended due to an exception: %s (%s)", e.what(), fileline);
}


bool StandardStateSystem::runState(const boost::shared_ptr<State> &state, bool canPause) {
    auto &current_state = getCurrentStateWeakRef();
    BOOST_SCOPE_EXIT(&current_state) {
        // Clear the current state on exit
        current_state.reset();
    } BOOST_SCOPE_EXIT_END
    
    current_state = state;
    auto current_state_shared = state;
    
    boost::weak_ptr<State> next_state;
    boost::shared_ptr<State> next_state_shared;
    
    bool in_action = false;
    bool in_transition = false;
    
    while (current_state_shared) {
        const bool canInterrupt = current_state_shared->isInterruptible();  // might not be an okay place to stop
        const bool shouldPause = canInterrupt && canPause && is_paused;
        
        if (canInterrupt &&
            (long(*state_system_mode) == IDLE ||      // hard stop
             long(*state_system_mode) == STOPPING))   // stop requested
        {
            break;
        }
        else if (in_transition ||  // waiting for the next state
                 shouldPause)      // paused
        {
            getClock()->yield();
            if (shouldPause) {
                continue;
            }
        }
        
        if (!in_transition) {
            in_action = true;
            
            try {
                current_state_shared->action();
            } catch (const std::exception &e) {
                failWithException(e, FILELINE);
                return false;
            }
            
            // finished performing action
            in_action = false;
        }
        
        if (!in_action) {
            in_transition = true;
            
            try {
                next_state = current_state_shared->next();
            } catch (const std::exception &e) {
                failWithException(e, FILELINE);
                return false;
            }
            
            next_state_shared = next_state.lock();
            if (!next_state_shared) {
                // no next state yet, sleep until the next tick
                continue;
            }
            
            if (next_state_shared == endState) {
                break;
            }
            
            current_state = next_state;
            current_state_shared = next_state_shared;
            
            next_state.reset();
            next_state_shared.reset();
            
            // finished transition
            in_transition = false;
        }
    }
    
    return true;
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
    
    mprintf(M_STATE_SYSTEM_MESSAGE_DOMAIN, "Starting state system....");
    (*state_system_mode) = RUNNING;
    
    if (!runState(current_experiment, true)) {
        // Execution failed.  Report that we're stopping the state system early.
        merror(M_PARADIGM_MESSAGE_DOMAIN, "Stopping state system");
    }
    
    (*state_system_mode) = STOPPING;
    
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
