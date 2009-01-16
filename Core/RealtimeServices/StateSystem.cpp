#include "Utilities.h"
#include "Scheduler.h"
#include "Experiment.h"
#include "StateSystem.h"
#include "EventBuffer.h"
#include "EventFactory.h" 
using namespace mw;
 
template <> shared_ptr<StateSystem> RegisteredSingleton<StateSystem>::singleton_instance = shared_ptr<StateSystem>();

StateSystem::StateSystem(const shared_ptr <Clock> &a_clock) {
	the_clock = a_clock;
}

StateSystem::~StateSystem() {}

void StateSystem::start(){}
    
void StateSystem::stop(){}

void StateSystem::pause(){}

bool StateSystem::isRunning(){ return false; }

bool StateSystem::isPaused(){ return false;  }

bool StateSystem::isInAction(){ return false; }

bool StateSystem::isInTransition(){ return false; }

void StateSystem::setInAction(bool isit){}

void StateSystem::setInTransition(bool isit){}

void StateSystem::sendSystemStateEvent() {
    if(GlobalBufferManager) {
		GlobalBufferManager->putEvent(EventFactory::currentExperimentState());
    }
}
        
weak_ptr<State> StateSystem::getCurrentState(){ return weak_ptr<State>(); }

void StateSystem::setCurrentState(weak_ptr<State> newcurrent){}

shared_ptr<Clock> StateSystem::getClock() const {
	return the_clock;
}
