#include "Utilities.h"
#include "Scheduler.h"
#include "Experiment.h"
#include "StateSystem.h"
#include "EventBuffer.h"
#include "SystemEventFactory.h" 


BEGIN_NAMESPACE_MW

 
//template <> shared_ptr<StateSystem> RegisteredSingleton<StateSystem>::singleton_instance = shared_ptr<StateSystem>();
SINGLETON_INSTANCE_STATIC_DECLARATION(StateSystem)

StateSystem::StateSystem(const shared_ptr <Clock> &a_clock) {
	the_clock = a_clock;
}

StateSystem::~StateSystem() {}

void StateSystem::start(){}
    
void StateSystem::stop(){}

void StateSystem::pause(){}

void StateSystem::resume(){}

bool StateSystem::isRunning(){ return false; }

bool StateSystem::isPaused(){ return false;  }

bool StateSystem::isInAction(){ return false; }

bool StateSystem::isInTransition(){ return false; }

//void StateSystem::setInAction(bool isit){}

//void StateSystem::setInTransition(bool isit){}

void StateSystem::sendSystemStateEvent() {
    if(global_outgoing_event_buffer) {
		global_outgoing_event_buffer->putEvent(SystemEventFactory::currentExperimentState());
    }
}
        
weak_ptr<State> StateSystem::getCurrentState(){ return weak_ptr<State>(); }

//void StateSystem::setCurrentState(weak_ptr<State> newcurrent){}

shared_ptr<Clock> StateSystem::getClock() const {
	return the_clock;
}


END_NAMESPACE_MW
