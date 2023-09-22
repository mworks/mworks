/**
 * States.cpp
 *
 * History:
 * ??/??/?? ??? - Created
 * Paul Janknas on 05/16/05 - fixed spacing, added functionality to state
 *                  class to add a name so users can see states by name.
 *
 * Copyright 2005 MIT. All rights reserved. 
 */
 
#include "States.h"
#include "Experiment.h"

#include <string>
#include "ComponentRegistry.h"
#include "ComponentInfo.h"
#include "ParameterValue.h"

#include "SequentialSelection.h"
#include "RandomWORSelection.h"
#include "RandomWithReplacementSelection.h"


BEGIN_NAMESPACE_MW


const boost::shared_ptr<State> & State::getEndState() {
    static const auto endState = boost::make_shared<State>();
    return endState;
}


void State::describeComponent(ComponentInfo &info) {
    Component::describeComponent(info);
    
    // "description" is no longer used but is still present in many old experiments
    info.addIgnoredParameter("description");
}


State::State() :
    experiment(GlobalCurrentExperiment)
{ }


State::State(const ParameterValueMap &parameters) :
    Component(parameters),
    experiment(GlobalCurrentExperiment)
{ }


void State::action() {
    currentState->setValue(getCompactID());
    
    if (auto experiment_shared = getExperiment()) {
        experiment_shared->announceLocalVariables();
    }
    
    updateCurrentScopedVariableContext();
}


boost::weak_ptr<State> State::next() {
    auto sharedParent = getParent();
    if (sharedParent) {
        sharedParent->updateCurrentScopedVariableContext();
    }
    
    reset();
    
    return (sharedParent ? sharedParent : getEndState());
}


void State::updateHierarchy() {
    if (auto sharedParent = getParent()) {
        setExperiment(sharedParent->getExperiment());
    }
}


boost::shared_ptr<ScopedVariableContext> State::getLocalScopedVariableContext() const {
    if (auto sharedParent = getParent()) {
        return sharedParent->getLocalScopedVariableContext();
    }
    return nullptr;
}


void State::updateCurrentScopedVariableContext() {
    if (auto sharedParent = getParent()) {
        sharedParent->updateCurrentScopedVariableContext();
    }
}


bool State::isInterruptible() const {
    if (!interruptible) {
        return false;
    }
    
    if (auto sharedParent = getParent()) {
        return sharedParent->isInterruptible();
    }
    
    return true;
}


const std::string ContainerState::INTERRUPTIBLE("interruptible");


void ContainerState::describeComponent(ComponentInfo &info) {
    State::describeComponent(info);
    info.addParameter(INTERRUPTIBLE, "YES");
}


ContainerState::ContainerState() {
	setName("ContainerState");
    requestVariableContext();
}


ContainerState::ContainerState(const ParameterValueMap &parameters) :
    State(parameters)
{
    setInterruptible(bool(parameters[INTERRUPTIBLE]));
    requestVariableContext();
}


void ContainerState::requestVariableContext() {
    if (auto sharedExperiment = getExperiment()) {
        local_variable_context = boost::make_shared<ScopedVariableContext>(sharedExperiment);
    }
}


void ContainerState::updateHierarchy() {
    State::updateHierarchy();
    
    if (!local_variable_context) {
        throw SimpleException(M_PARADIGM_MESSAGE_DOMAIN, "Invalid variable context in state object");
    }
    
    if (auto sharedParent = getParent()) {
        if (auto parentContext = sharedParent->getLocalScopedVariableContext()) {
            local_variable_context->inheritFrom(parentContext);
        }
    }
    
    auto self_ptr = component_shared_from_this<State>();
    for (auto &child : *list) {
        // recurse down the hierarchy
        child->setParent(self_ptr);
        child->updateHierarchy();
    }
}


void ContainerState::reset() {
    accessed = false;
    
    for (auto &child : *list) {
        // call recursively down the experiment hierarchy
        child->reset();
    }
    
    State::reset();
}


boost::shared_ptr<ScopedVariableContext> ContainerState::getLocalScopedVariableContext() const {
    return local_variable_context;
}


void ContainerState::updateCurrentScopedVariableContext() {
    auto environment_shared = getExperiment();
    if (!environment_shared) {
        throw SimpleException(M_PARADIGM_MESSAGE_DOMAIN,
                              "Cannot update scoped variable context without a valid environment");
    }
    environment_shared->setCurrentContext(local_variable_context);
    
    if (auto parent_shared = getParent()) {
        if (auto parentContext = parent_shared->getLocalScopedVariableContext()) {
            local_variable_context->inheritFrom(parentContext);
        }
    }
}


void ContainerState::addChild(std::map<std::string, std::string> parameters,
                              ComponentRegistry *reg,
                              boost::shared_ptr<mw::Component> child)
{
    auto state = boost::dynamic_pointer_cast<State>(child);
    
    if (!state) {
        throw SimpleException(M_PARADIGM_MESSAGE_DOMAIN,
                              "Attempt to add non-paradigm component object as child of a paradigm component");
    }
    
    list->push_back(state);
    state->setParent(component_shared_from_this<State>());
    state->updateHierarchy();
}


const std::string ListState::SELECTION("selection");
const std::string ListState::NSAMPLES("nsamples");
const std::string ListState::SAMPLING_METHOD("sampling_method");


void ListState::describeComponent(ComponentInfo &info) {
    ContainerState::describeComponent(info);
    info.addParameter(SELECTION, "sequential");
    info.addParameter(NSAMPLES, "1");
    info.addParameter(SAMPLING_METHOD, "cycles");
}


ListState::ListState() :
    selection_type(M_SEQUENTIAL),
    nsamples(1),
    sampling_method(M_CYCLES)
{ }


ListState::ListState(const ParameterValueMap &parameters) :
    ContainerState(parameters),
    selection_type(parameters[SELECTION]),
    nsamples(parameters[NSAMPLES]),
    sampling_method(parameters[SAMPLING_METHOD])
{
    if (nsamples < 1) {
        throw SimpleException(M_PARADIGM_MESSAGE_DOMAIN, "\"nsamples\" must be greater than or equal to 1");
    }
}


boost::weak_ptr<State> ListState::next() {
    if (!selection) {
        throw SimpleException(M_PARADIGM_MESSAGE_DOMAIN, "Attempt to draw from an invalid selection object");
    }
    
    if (hasMoreChildrenToRun()) {
        int index;
        try {
            index = selection->draw();
        } catch (const std::exception &e) {
            index = -1; // just a bandaid for now
        }
        
        boost::shared_ptr<State> thestate;
        if ((index < 0) || !(thestate = getList()[index])) {
            mwarning(M_PARADIGM_MESSAGE_DOMAIN,
                     "List state returned invalid state at index %d",
                     index);
            return State::next();
        }
        
        auto thestate_parent = thestate->getParent();
        if (thestate_parent.get() != this) {
            // this ensures that we find our way back,
            // even if something is screwed up
            thestate->setParent(component_shared_from_this<State>());
            thestate->updateHierarchy(); // TODO: might want to do this differently
        }
        
        thestate->updateCurrentScopedVariableContext();
        return thestate;
    }
    
    return State::next();
}


void ListState::reset() {
    if (selection) {
        selection->reset();
    }
    ContainerState::reset();
}


void ListState::finalize(std::map<std::string, std::string> parameters, ComponentRegistry *reg) {
    long N = nsamples;
    if (sampling_method == M_CYCLES) {
        N *= getNItems();
    }
    
    boost::shared_ptr<Selection> newSelection;
    
    switch (selection_type) {
        case M_SEQUENTIAL:
        case M_SEQUENTIAL_ASCENDING:
            newSelection = boost::make_shared<SequentialSelection>(N);
            break;
            
        case M_SEQUENTIAL_DESCENDING:
            newSelection = boost::make_shared<SequentialSelection>(N, false);
            break;
            
        case M_RANDOM_WITH_REPLACEMENT:
            newSelection = boost::make_shared<RandomWithReplacementSelection>(N);
            break;
            
        case M_RANDOM_WOR:
            newSelection = boost::make_shared<RandomWORSelection>(N);
            break;
    }
    
    attachSelection(newSelection);
}


END_NAMESPACE_MW
