// * Paul Jankunas on 1/24/06 - Adding virtual destructor.
// Paul Jankunas on 1/31/06 - Added methods to send events when the system
// starts, stops pauses and unpauses use these in your plugins.
#ifndef STATE_SYSTEM_H
#define STATE_SYSTEM_H

#include "MonkeyWorksTypes.h"
#include "States.h"
#include "Clock.h"
#include "boost/enable_shared_from_this.hpp"
//#include "States.h"


namespace mw {
class StateSystem : public mw::Component, public enable_shared_from_this<StateSystem>, public RegisteredSingleton<StateSystem> {
protected:
	shared_ptr <Clock> the_clock;
	
public:
    
	StateSystem(const shared_ptr <Clock> &a_clock);
	
	virtual ~StateSystem();
	
	
	/*StateSystem(Experiment *exp);
	 
	 virtual void setExperiment(Experiment *exp);
	 virtual Experiment *getExperiment();*/
    
	virtual void start();
	virtual void stop();
	virtual void pause();
	
	virtual bool isRunning();
	virtual bool isPaused();
	
	virtual bool isInAction();
	virtual bool isInTransition();
	
	virtual void setInAction(bool);
	virtual void setInTransition(bool);
	
	// use these to send the proper events at the proper times
	virtual void sendSystemStateEvent();
	
	weak_ptr<State> getCurrentState();
	void setCurrentState(weak_ptr<State> current);
	shared_ptr<Clock> getClock() const;
};

}


#endif

