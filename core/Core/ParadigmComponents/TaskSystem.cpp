//
//  TaskSystem.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 9/20/23.
//

#include "TaskSystem.hpp"

#include <boost/foreach.hpp>

#include "Actions.hpp"


BEGIN_NAMESPACE_MW


/****************************************************************
 *                 TransitionCondition Methods
 ****************************************************************/	
TransitionCondition::TransitionCondition(shared_ptr<Variable> v1, 
										   weak_ptr<State> trans) {
	//    mprintf("Instantiating conditional transitions");
	condition = v1;
	always_go = false;
	if(trans.expired()) {
		throw SimpleException("An invalid transition has been created that, if used, will likely result in an infinite loop");
	} else {
		transition = trans;
	}
	name = "Transition: TransitionCondition";
}		

TransitionCondition::TransitionCondition(weak_ptr<State> trans) { 
	//condition = NULL;
	always_go = true;
	if(trans.expired()) {
		throw SimpleException("An invalid transition has been created that, if used, will likely result in an infinite loop");
	} else {
		transition = trans;
	}
}

TransitionCondition::~TransitionCondition() {
    //transition = NULL;
    //owner = NULL;
}

weak_ptr<State> TransitionCondition::execute() {
    
	
	if(always_go){
		
		if(!transition.expired()){
			currentState->setValue(getCompactID());
            return transition;
		} else {
			// TODO: better throw
			throw SimpleException("Attempt to advance to an invalid transition state");
		}
	} else if((bool)(*condition)) {
		//		fprintf(stderr, "===> Going because the condition evaluated true\n");
		//		fflush(stderr); 
		
		if(!transition.expired()){
			currentState->setValue(getCompactID());
			return transition;
		} else {
			// TODO: better throw
			throw SimpleException("Attempt to advance to an invalid transition state");
		}
    } else {
		return weak_ptr<State>();
	}
}

void TransitionCondition::setOwner(weak_ptr<State> _owner) {
	owner = _owner;
}

weak_ptr<State> TransitionCondition::getOwner() {
	return owner;
}

weak_ptr<State>  TransitionCondition::getTransition() {
    return transition;
}

shared_ptr<mw::Component> TransitionFactory::createObject(std::map<std::string, std::string> parameters,
														ComponentRegistry *reg) {
	
	REQUIRE_ATTRIBUTES(parameters, "type");
	
	shared_ptr<mw::Component> newTransition;
	    
	if(boost::algorithm::to_lower_copy(parameters.find("type")->second) == "yield") {
		newTransition = shared_ptr<mw::Component>(new YieldToParent());	 
	} else {
		// if it doesn't yield it needs a target
		REQUIRE_ATTRIBUTES(parameters, "type", "target");
		shared_ptr<State> target = reg->getObject<State>(parameters.find("target")->second, parameters["parent_scope"]);
		
		checkAttribute(target, parameters["reference_id"], "target", parameters.find("target")->second);		
		
		string type = boost::algorithm::to_lower_copy(parameters.find("type")->second);
		if(type == "conditional") {
			REQUIRE_ATTRIBUTES(parameters, "type", "target", "condition");
			shared_ptr<Variable> condition = reg->getVariable(parameters.find("condition")->second);			
			
			checkAttribute(condition, parameters["reference_id"], "condition", parameters.find("condition")->second);		
			
			checkAttribute(target, parameters["reference_id"], "target", parameters.find("target")->second);		
			
			
			newTransition = shared_ptr<mw::Component>(new TransitionCondition(condition, target));
		} else if(type == "timer_expired") {
			REQUIRE_ATTRIBUTES(parameters, "type", "target", "timer");
			shared_ptr<Timer> timer = reg->getObject<Timer>(parameters.find("timer")->second);
			
			checkAttribute(timer, parameters["reference_id"], "timer", parameters.find("timer")->second);		
			
			checkAttribute(target, parameters["reference_id"], "target", parameters.find("target")->second);		
			
			
			newTransition = shared_ptr<mw::Component>(new TransitionIfTimerExpired(timer, target));
		} else if(type == "direct" || type == "") {
			REQUIRE_ATTRIBUTES(parameters, "type", "target");
			checkAttribute(target, parameters["reference_id"], "target", parameters.find("target")->second);		
			
			newTransition = shared_ptr<mw::Component>(new TransitionCondition(target));
        } else if (type == "goto") {
            if (parameters.find("when") == parameters.end()) {
                newTransition = boost::make_shared<TransitionCondition>(target);
            } else {
                auto when = reg->getVariable(parameters.find("when")->second);
                checkAttribute(when, parameters["reference_id"], "when", parameters.find("when")->second);
                newTransition = boost::make_shared<TransitionCondition>(when, target);
            }
		} else {
			throw SimpleException("Illegal type for transition");
		}
	}		
	return newTransition;
}

/****************************************************************
 *                 TransitionIfTimerExpired Methods
 ****************************************************************/	
TransitionIfTimerExpired::TransitionIfTimerExpired(shared_ptr<Timer> _timer, 
													 weak_ptr<State> trans)
: TransitionCondition(trans) {
	timer = _timer;
    always_go = false;
	name = "Transition: TransitionIfTimerExpired";
}


weak_ptr<State> TransitionIfTimerExpired::execute() {
    
	if(timer->hasExpired()) {
		//fprintf(stderr, "===> Going because timer expired\n");
		//fflush(stderr);
		if(!transition.expired()){
			currentState->setValue(getCompactID());
            return transition;
		} else {
			// TODO: better throw
			throw SimpleException("Attempt to advance to an invalid transition state");
		}
    } else {
        return weak_ptr<State>();
    }
}


/****************************************************************
 *                 YieldToParent Methods
 ****************************************************************/
YieldToParent::YieldToParent() 
: TransitionCondition(shared_ptr<State>(GlobalCurrentExperiment)) {
	// surely not the right place to go, but might avoid a crash    
    // mprintf("YieldToParent Constructor called");
	name = "Transition: YieldToParent";
}

YieldToParent::~YieldToParent() {
	
}

weak_ptr<State> YieldToParent::execute() {
	
	currentState->setValue(getCompactID());
	//currentState->setValue(name);
	//	fprintf(stderr, "===> Yielding to parent\n");
	//	fflush(stderr);
	
	shared_ptr<State> owner_shared(owner);
	
  	if(owner_shared == NULL) {
		merror(M_PARADIGM_MESSAGE_DOMAIN, 
			   "Attempting to yield in an owner-less transition");
		
		// TODO: is this implicit cast kosher?
		return weak_ptr<State>(GlobalCurrentExperiment);
	}
	
	shared_ptr<State> parent = owner_shared->getParent();
	if (!parent) {
		merror(M_PARADIGM_MESSAGE_DOMAIN, 
			   "Attempting to yield in an owner-less transition");
		return GlobalCurrentExperiment;
	}
	return parent;
}

/****************************************************************
 *                 TaskSystemState Methods
 ****************************************************************/


void TaskSystemState::describeComponent(ComponentInfo &info) {
    ContainerState::describeComponent(info);
    info.setSignature("task_system_state");
}


TaskSystemState::TaskSystemState() {
	setTag("TaskSystemState");
}


TaskSystemState::TaskSystemState(const ParameterValueMap &parameters) :
    ContainerState(parameters)
{ }


shared_ptr<mw::Component> TaskSystemState::createInstanceObject(){
    shared_ptr<TaskSystemState> new_state = clone<TaskSystemState>();
    new_state->transition_list = transition_list;
	return new_state;
}


weak_ptr<State> TaskSystemState::next() {
    if (currentActionIndex < getList().size()) {
        shared_ptr<State> action = getList()[currentActionIndex++];
        
        shared_ptr<State> actionParent(action->getParent());
        if (actionParent.get() != this) {
            action->setParent(component_shared_from_this<State>());
            action->updateHierarchy();
        }
        
        action->updateCurrentScopedVariableContext();
        
        return action;
    }
    
    weak_ptr<State> trans;
	if(transition_list->size() == 0) {
		merror(M_STATE_SYSTEM_MESSAGE_DOMAIN, "No valid transitions.  Ending experiment.");
		trans = weak_ptr<State>(GlobalCurrentExperiment);
		return trans;
	}			
	
    BOOST_FOREACH( shared_ptr<TransitionCondition> condition, *transition_list ) {
		trans = condition->execute();
        shared_ptr<State> trans_shared = trans.lock();
		if(trans_shared) {
			shared_ptr<State> parent_shared(getParent());
			if(trans_shared.get() != parent_shared.get()){
				trans_shared->setParent(parent_shared); // TODO: this gets set WAY too many times
                trans_shared->updateHierarchy();
			}
			
			trans_shared->updateCurrentScopedVariableContext();
            reset();
			return trans;
		}
	}	
	return weak_ptr<State>();
}


void TaskSystemState::reset() {
    ContainerState::reset();
    currentActionIndex = 0;
}


void TaskSystemState::addChild(std::map<std::string, std::string> parameters,
								ComponentRegistry *reg,
								shared_ptr<mw::Component> comp){
	
	shared_ptr<Action> as_action = boost::dynamic_pointer_cast<Action, mw::Component>(comp);
	if(as_action != NULL){
        if (!transition_list->empty()) {
            throw SimpleException(M_PARADIGM_MESSAGE_DOMAIN,
                                  "Actions must be placed before transitions in task system state");
        }
        return ContainerState::addChild(parameters, reg, comp);
	}
	
	shared_ptr<TransitionCondition> as_transition = boost::dynamic_pointer_cast<TransitionCondition, mw::Component>(comp);
	if(as_transition != NULL){
		return addTransition(as_transition);
	}
	
	throw SimpleException("Attempting to add something (" + comp->getTag() + ") to task state (" + this->getTag() + ") that is not a transition or action");
}


void TaskSystemState::addTransition(shared_ptr<TransitionCondition> trans) {
	if(!trans) {
		mprintf("Attempt to add NULL transition");
		return;
	}
	trans->setOwner(component_shared_from_this<State>());
	transition_list->push_back(trans);
}


/****************************************************************
 *                 TaskSystem Methods
 ****************************************************************/


void TaskSystem::describeComponent(ComponentInfo &info) {
    ContainerState::describeComponent(info);
    info.setSignature("task_system");
}


TaskSystem::TaskSystem() {
	setTag("TaskSystem");
}


TaskSystem::TaskSystem(const ParameterValueMap &parameters) :
    ContainerState(parameters)
{ }


shared_ptr<mw::Component> TaskSystem::createInstanceObject(){
    shared_ptr<TaskSystem> new_state(clone<TaskSystem>());
	
    shared_ptr<ScopedVariableEnvironment> env_shared = getExperiment();
	if(env_shared){
		shared_ptr<ScopedVariableContext> con = env_shared->createNewContext();
		
		new_state->setLocalScopedVariableContext(con);
	} else {
		merror(M_PARADIGM_MESSAGE_DOMAIN,
			   "Attempt to clone a state without an associated scoped environment");
	}
    
    return new_state;
	
}


void TaskSystem::action() {
    ContainerState::action();
	updateHierarchy();  // TODO: need to rethink how all of this is working...
}


weak_ptr<State> TaskSystem::next() {
	if (accessed) {
        return State::next();
	} else {
		if(getList().size() > 0) {
			accessed = true;
			//mprintf("Trial execution triggered");
			return getList()[0];
		} else {
			mwarning(M_PARADIGM_MESSAGE_DOMAIN,
					 "Task system contains no states");
            return State::next();
		}
	}
}


void TaskSystem::addChild(std::map<std::string, std::string> parameters,
                          ComponentRegistry *reg,
                          shared_ptr<mw::Component> comp)
{
    ContainerState::addChild(parameters, reg, comp);
    string full_tag = parameters["parent_tag"];
    full_tag += "/";
    full_tag += parameters["child_tag"];
}


END_NAMESPACE_MW
