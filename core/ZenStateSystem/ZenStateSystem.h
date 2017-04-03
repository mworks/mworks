/*
 *  ZenStateSystem.h
 *  MWorksCore
 *
 *  Created by David Cox on 6/16/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef STANDARD_STATE_SYSTEM_H
#define STANDARD_STATE_SYSTEM_H

#include <thread>

#include <boost/thread/mutex.hpp>

#include <MWorksCore/StateSystem.h>


BEGIN_NAMESPACE_MW


class StandardStateSystem : public mw::Component, public StateSystem {
    
public:
    StandardStateSystem(const shared_ptr <Clock> &a_clock);
    ~StandardStateSystem();
    
    virtual void start();
    virtual void stop();
    virtual void pause();
    virtual void resume();
    
    virtual bool isRunning();
    virtual bool isPaused();
    
    virtual bool isInAction();
    virtual bool isInTransition();
    
    //virtual void setInAction(bool);
    //virtual void setInTransition(bool);
    
    virtual weak_ptr<State> getCurrentState();
    //virtual void setCurrentState(weak_ptr<State> new_current);
    
private:
    void run();
    
    boost::mutex state_system_mutex;
    std::thread state_system_thread;
    
    bool in_action, in_transition, is_running, is_paused;
    weak_ptr<State> current_state;
    
};


END_NAMESPACE_MW


#endif



























