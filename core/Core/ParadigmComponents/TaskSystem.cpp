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
 *                    TaskSystem Methods
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
    if (!accessed) {
        if (getList().empty()) {
            mwarning(M_PARADIGM_MESSAGE_DOMAIN, "Task contains no states");
        } else {
            nextState = boost::dynamic_pointer_cast<TaskSystemState>(getList().front());
        }
        accessed = true;
    }
}


boost::weak_ptr<State> TaskSystem::next() {
    if (nextState) {
        if (nextState->getParent().get() != this) {
            nextState->setParent(component_shared_from_this<State>());
            nextState->updateHierarchy();
        }
        return nextState;
    }
    return ContainerState::next();
}


void TaskSystem::reset() {
    nextState.reset();
    ContainerState::reset();
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


/****************************************************************
 *                  TaskSystemState Methods
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
        if (action->getParent().get() != this) {
            action->setParent(component_shared_from_this<State>());
            action->updateHierarchy();
        }
        return action;
    }
    
    if (transition_list->empty()) {
        merror(M_STATE_SYSTEM_MESSAGE_DOMAIN, "No valid transitions.  Ending experiment.");
        StateSystem::instance()->stop();
        return ContainerState::next();
    }
    
    for (auto &transition : *transition_list) {
        if (transition->evaluate()) {
            currentState->setValue(transition->getCompactID());
            if (auto parentTask = boost::dynamic_pointer_cast<TaskSystem>(getParent())) {
                parentTask->setNextState(transition->getTarget());
                return ContainerState::next();
            }
            throw SimpleException(M_PARADIGM_MESSAGE_DOMAIN, "Parent of task state is not a task");
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
                                  "Actions must be placed before transitions in task state");
        }
        ContainerState::addChild(parameters, reg, comp);
        return;
    }
    
    auto as_transition = boost::dynamic_pointer_cast<Transition>(comp);
    if (as_transition) {
        transition_list->push_back(as_transition);
        return;
    }
    
    throw SimpleException(M_PARADIGM_MESSAGE_DOMAIN,
                          "Attempting to add something (" + comp->getTag() + ") to task state (" + getTag() + ") that is not a transition or action");
}


/****************************************************************
 *                   Transition Methods
 ****************************************************************/


const std::string Transition::TARGET("target");


Transition::Transition(const ParameterValueMap &parameters) :
    Component(parameters)
{ }


/****************************************************************
 *                 DirectTransition Methods
 ****************************************************************/


void DirectTransition::describeComponent(ComponentInfo &info) {
    Transition::describeComponent(info);
    info.setSignature("transition/direct");
    info.addParameter(TARGET);
}


bool DirectTransition::evaluate() {
    return true;
}


/****************************************************************
 *               ConditionalTransition Methods
 ****************************************************************/


const std::string ConditionalTransition::CONDITION("condition");


void ConditionalTransition::describeComponent(ComponentInfo &info) {
    Transition::describeComponent(info);
    info.setSignature("transition/conditional");
    info.addParameter(CONDITION);
    info.addParameter(TARGET);
}


ConditionalTransition::ConditionalTransition(const ParameterValueMap &parameters) :
    Transition(parameters),
    condition(parameters[CONDITION])
{ }


bool ConditionalTransition::evaluate() {
    return condition->getValue().getBool();
}


/****************************************************************
 *              TimerExpiredTransition Methods
 ****************************************************************/


const std::string TimerExpiredTransition::TIMER("timer");


void TimerExpiredTransition::describeComponent(ComponentInfo &info) {
    Transition::describeComponent(info);
    info.setSignature("transition/timer_expired");
    info.addParameter(TIMER);
    info.addParameter(TARGET);
}


TimerExpiredTransition::TimerExpiredTransition(const ParameterValueMap &parameters) :
    Transition(parameters),
    timer(parameters[TIMER])
{ }


bool TimerExpiredTransition::evaluate() {
    return timer->hasExpired();
}


/****************************************************************
 *                  GotoTransition Methods
 ****************************************************************/


const std::string GotoTransition::WHEN("when");


void GotoTransition::describeComponent(ComponentInfo &info) {
    Transition::describeComponent(info);
    info.setSignature("transition/goto");
    info.addParameter(TARGET);
    info.addParameter(WHEN, false);
}


GotoTransition::GotoTransition(const ParameterValueMap &parameters) :
    Transition(parameters),
    when(optionalVariable(parameters[WHEN]))
{ }


bool GotoTransition::evaluate() {
    return (!when || when->getValue().getBool());
}


/****************************************************************
 *                  YieldTransition Methods
 ****************************************************************/


void YieldTransition::describeComponent(ComponentInfo &info) {
    Transition::describeComponent(info);
    info.setSignature("transition/yield");
}


bool YieldTransition::evaluate() {
    return true;
}


END_NAMESPACE_MW
