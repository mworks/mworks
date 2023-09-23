//
//  TaskSystem.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 9/20/23.
//

#include "TaskSystem.hpp"

#include "Actions.hpp"


BEGIN_NAMESPACE_MW


/****************************************************************
 *                 TransitionCondition Methods
 ****************************************************************/	
TransitionCondition::TransitionCondition(boost::shared_ptr<Variable> v1,
										   boost::weak_ptr<State> trans) {
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

TransitionCondition::TransitionCondition(boost::weak_ptr<State> trans) {
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

boost::weak_ptr<State> TransitionCondition::execute() {
    
	
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
		return boost::weak_ptr<State>();
	}
}

void TransitionCondition::setOwner(boost::weak_ptr<State> _owner) {
	owner = _owner;
}

boost::weak_ptr<State> TransitionCondition::getOwner() {
	return owner;
}

boost::weak_ptr<State>  TransitionCondition::getTransition() {
    return transition;
}

boost::shared_ptr<Component> TransitionFactory::createObject(std::map<std::string, std::string> parameters,
														ComponentRegistry *reg) {
	
	REQUIRE_ATTRIBUTES(parameters, "type");
	
	boost::shared_ptr<Component> newTransition;
	    
	if(boost::algorithm::to_lower_copy(parameters.find("type")->second) == "yield") {
		newTransition = boost::shared_ptr<Component>(new YieldToParent());
	} else {
		// if it doesn't yield it needs a target
		REQUIRE_ATTRIBUTES(parameters, "type", "target");
		boost::shared_ptr<State> target = reg->getObject<State>(parameters.find("target")->second, parameters["parent_scope"]);
		
		checkAttribute(target, parameters["reference_id"], "target", parameters.find("target")->second);		
		
		string type = boost::algorithm::to_lower_copy(parameters.find("type")->second);
		if(type == "conditional") {
			REQUIRE_ATTRIBUTES(parameters, "type", "target", "condition");
			boost::shared_ptr<Variable> condition = reg->getVariable(parameters.find("condition")->second);
			
			checkAttribute(condition, parameters["reference_id"], "condition", parameters.find("condition")->second);		
			
			checkAttribute(target, parameters["reference_id"], "target", parameters.find("target")->second);		
			
			
			newTransition = boost::shared_ptr<Component>(new TransitionCondition(condition, target));
		} else if(type == "timer_expired") {
			REQUIRE_ATTRIBUTES(parameters, "type", "target", "timer");
			boost::shared_ptr<Timer> timer = reg->getObject<Timer>(parameters.find("timer")->second);
			
			checkAttribute(timer, parameters["reference_id"], "timer", parameters.find("timer")->second);		
			
			checkAttribute(target, parameters["reference_id"], "target", parameters.find("target")->second);		
			
			
			newTransition = boost::shared_ptr<Component>(new TransitionIfTimerExpired(timer, target));
		} else if(type == "direct" || type == "") {
			REQUIRE_ATTRIBUTES(parameters, "type", "target");
			checkAttribute(target, parameters["reference_id"], "target", parameters.find("target")->second);		
			
			newTransition = boost::shared_ptr<Component>(new TransitionCondition(target));
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
TransitionIfTimerExpired::TransitionIfTimerExpired(boost::shared_ptr<Timer> _timer,
													 boost::weak_ptr<State> trans)
: TransitionCondition(trans) {
	timer = _timer;
    always_go = false;
	name = "Transition: TransitionIfTimerExpired";
}


boost::weak_ptr<State> TransitionIfTimerExpired::execute() {
    
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
        return boost::weak_ptr<State>();
    }
}


/****************************************************************
 *                 YieldToParent Methods
 ****************************************************************/
YieldToParent::YieldToParent() 
: TransitionCondition(boost::shared_ptr<State>(GlobalCurrentExperiment)) {
	// surely not the right place to go, but might avoid a crash    
    // mprintf("YieldToParent Constructor called");
	name = "Transition: YieldToParent";
}

YieldToParent::~YieldToParent() {
	
}

boost::weak_ptr<State> YieldToParent::execute() {
	
	currentState->setValue(getCompactID());
	//currentState->setValue(name);
	//	fprintf(stderr, "===> Yielding to parent\n");
	//	fflush(stderr);
	
	boost::shared_ptr<State> owner_shared(owner);
	
  	if(owner_shared == NULL) {
		merror(M_PARADIGM_MESSAGE_DOMAIN, 
			   "Attempting to yield in an owner-less transition");
		
		// TODO: is this implicit cast kosher?
		return boost::weak_ptr<State>(GlobalCurrentExperiment);
	}
	
	boost::shared_ptr<State> parent = owner_shared->getParent();
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


boost::shared_ptr<Component> TaskSystemState::createInstanceObject() {
    auto new_state = clone<TaskSystemState>();
    new_state->transition_list = transition_list;
    return new_state;
}


boost::weak_ptr<State> TaskSystemState::next() {
    if (currentActionIndex < getList().size()) {
        auto action = getList()[currentActionIndex++];
        
        auto actionParent = action->getParent();
        if (actionParent.get() != this) {
            action->setParent(component_shared_from_this<State>());
            action->updateHierarchy();
        }
        
        action->updateCurrentScopedVariableContext();
        
        return action;
    }
    
    if (transition_list->empty()) {
        merror(M_STATE_SYSTEM_MESSAGE_DOMAIN, "No valid transitions.  Ending experiment.");
        return boost::weak_ptr<State>(GlobalCurrentExperiment);
    }
    
    for (auto &condition : *transition_list) {
        auto trans = condition->execute();
        if (auto trans_shared = trans.lock()) {
            auto parent_shared = getParent();
            if (trans_shared != parent_shared) {
                auto trans_parent_shared = trans_shared->getParent();
                if (trans_parent_shared != parent_shared) {
                    trans_shared->setParent(parent_shared);
                    trans_shared->updateHierarchy();
                }
            }
            
            trans_shared->updateCurrentScopedVariableContext();
            reset();
            return trans;
        }
    }
    
    return boost::weak_ptr<State>();
}


void TaskSystemState::reset() {
    currentActionIndex = 0;
    ContainerState::reset();
}


void TaskSystemState::addChild(std::map<std::string, std::string> parameters,
                               ComponentRegistry *reg,
                               boost::shared_ptr<Component> comp)
{
    auto as_action = boost::dynamic_pointer_cast<Action>(comp);
    if (as_action) {
        if (!transition_list->empty()) {
            throw SimpleException(M_PARADIGM_MESSAGE_DOMAIN,
                                  "Actions must be placed before transitions in task system state");
        }
        ContainerState::addChild(parameters, reg, comp);
        return;
    }
    
    auto as_transition = boost::dynamic_pointer_cast<TransitionCondition>(comp);
    if (as_transition) {
        as_transition->setOwner(component_shared_from_this<State>());
        transition_list->push_back(as_transition);
        return;
    }
    
    throw SimpleException(M_PARADIGM_MESSAGE_DOMAIN,
                          "Attempting to add something (" + comp->getTag() + ") to task state (" + getTag() + ") that is not a transition or action");
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


boost::shared_ptr<Component> TaskSystem::createInstanceObject() {
    return clone<TaskSystem>();
}


void TaskSystem::action() {
    ContainerState::action();
    updateHierarchy();  // TODO: need to rethink how all of this is working...
}


boost::weak_ptr<State> TaskSystem::next() {
    if (!accessed) {
        if (getList().empty()) {
            mwarning(M_PARADIGM_MESSAGE_DOMAIN, "Task system contains no states");
        } else {
            accessed = true;
            return getList()[0];
        }
    }
    return State::next();
}


void TaskSystem::addChild(std::map<std::string, std::string> parameters,
                          ComponentRegistry *reg,
                          boost::shared_ptr<Component> comp)
{
    if (auto tss = boost::dynamic_pointer_cast<TaskSystemState>(comp)) {
        ContainerState::addChild(parameters, reg, comp);
        return;
    }
    throw SimpleException(M_PARADIGM_MESSAGE_DOMAIN,
                          "Attempting to add something (" + comp->getTag() + ") to task (" + getTag() + ") that is not a task state");
}


END_NAMESPACE_MW
