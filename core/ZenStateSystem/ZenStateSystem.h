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

#include <MWorksCore/StateSystem.h>


BEGIN_NAMESPACE_MW


class StandardStateSystem : public Component, public StateSystem {
    
public:
    explicit StandardStateSystem(const boost::shared_ptr<Clock> &clock);
    ~StandardStateSystem();
    
    void start() override;
    void stop() override;
    void pause() override;
    void resume() override;
    
    bool isRunning() override { return is_running; }
    bool isPaused() override { return is_paused; }
    
    boost::shared_ptr<State> getCurrentState() override;
    
private:
    void run();
    
    using lock_guard = std::lock_guard<std::mutex>;
    lock_guard::mutex_type state_system_mutex;
    std::thread state_system_thread;
    
    std::atomic_bool is_running, is_paused;
    static_assert(decltype(is_running)::is_always_lock_free);
    boost::weak_ptr<State> current_state;
    
};


END_NAMESPACE_MW


#endif /* STANDARD_STATE_SYSTEM_H */
