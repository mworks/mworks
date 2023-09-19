#ifndef STATE_SYSTEM_H
#define STATE_SYSTEM_H

#include "Clock.h"
#include "States.h"


BEGIN_NAMESPACE_MW


class StateSystem {
    
private:
    const boost::shared_ptr<Clock> clock;
    
protected:
    static void sendSystemStateEvent();
    
    const boost::shared_ptr<Clock> & getClock() const { return clock; }
    
public:
    explicit StateSystem(const boost::shared_ptr<Clock> &clock) : clock(clock) { }
    
    virtual ~StateSystem() { }
    
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void pause() = 0;
    virtual void resume() = 0;
    
    virtual bool isRunning() = 0;
    virtual bool isPaused() = 0;
    
    virtual boost::shared_ptr<State> getCurrentState() = 0;
    virtual bool runState(const boost::shared_ptr<State> &state) = 0;
    
    REGISTERED_SINGLETON_CODE_INJECTION(StateSystem)
    
};


END_NAMESPACE_MW


#endif /* STATE_SYSTEM_H */
