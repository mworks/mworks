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

#include <boost/foreach.hpp>


BEGIN_NAMESPACE_MW


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
	
    shared_ptr<Experiment> experiment_shared(getExperiment());
	if (experiment_shared) {
		experiment_shared->announceLocalVariables();
	}
	
	updateCurrentScopedVariableContext();
}


weak_ptr<State> State::next() {
    shared_ptr<State> sharedParent = getParent();
    if (!sharedParent) {
        throw SimpleException("Internal error: state has no parent");
    }
    
    sharedParent->updateCurrentScopedVariableContext();
    reset();
    
    return sharedParent;
}


void State::updateHierarchy() {
    if (auto sharedParent = getParent()) {
        setExperiment(sharedParent->getExperiment());
    }
}


shared_ptr<ScopedVariableContext> State::getLocalScopedVariableContext() const {
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
    
    shared_ptr<State> sharedParent(getParent());
    if (sharedParent) {
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
        local_variable_context = sharedExperiment->createNewContext();
    }
}


void ContainerState::updateHierarchy() {
    State::updateHierarchy();
    
    if (!local_variable_context) {
        merror(M_SYSTEM_MESSAGE_DOMAIN, "Invalid variable context in state object");
        local_variable_context = shared_ptr<ScopedVariableContext>(new ScopedVariableContext(NULL));
    }
    
    if (auto sharedParent = getParent()) {
        if (auto parentContext = sharedParent->getLocalScopedVariableContext()) {
            local_variable_context->inheritFrom(parentContext);
        } else {
            merror(M_SYSTEM_MESSAGE_DOMAIN, "Invalid parent variable context");
            //throw("I don't know what will happen here yet...");
            //local_variable_context = new ScopedVariableContext();// DEAL WITH THIS EVENTUALLY
        }
    }
    
    shared_ptr<State> self_ptr = component_shared_from_this<State>();
	
    BOOST_FOREACH( shared_ptr<State> child, *list ) {
        // recurse down the hierarchy
        child->setParent(self_ptr);
        child->updateHierarchy(); 
    }
}


void ContainerState::reset() {
    accessed = false;
    
    BOOST_FOREACH( shared_ptr<State> child, *list ) {
        // call recursively down the experiment hierarchy
        child->reset();
    }
}


shared_ptr<ScopedVariableContext> ContainerState::getLocalScopedVariableContext() const {
    return local_variable_context;
}


void ContainerState::updateCurrentScopedVariableContext() {
    shared_ptr<ScopedVariableEnvironment> environment_shared = getExperiment();
    if (environment_shared) {
        environment_shared->setCurrentContext(local_variable_context);
        
        shared_ptr<State> parent_shared(getParent());
        if (parent_shared) {
            shared_ptr<ScopedVariableContext> parentContext = parent_shared->getLocalScopedVariableContext();
            if (parentContext) {
                int numScopedVars = local_variable_context->getNFields();
                for (int i=0; i<numScopedVars; ++i) {
                    if (local_variable_context->getTransparency(i) == M_TRANSPARENT) {
                        local_variable_context->setWithTransparency(i, parentContext->get(i));
                    }
                }
            }
        }
    } else {
        // TODO: better throw
        throw SimpleException("Cannot update scoped variable context without a valid environment");
    }
}


void ContainerState::addChild(std::map<std::string, std::string> parameters,
                              ComponentRegistry *reg,
                              shared_ptr<mw::Component> child)
{
    auto state = boost::dynamic_pointer_cast<State>(child);
    
    if (!state) {
        throw SimpleException("Attempt to add non-paradigm component object as child of a paradigm component");
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


template<>
SelectionType ParameterValue::convert(const std::string &s, ComponentRegistryPtr reg) {
    std::string selection_string(boost::algorithm::to_lower_copy(s));

    if (selection_string == "sequential") {
        return M_SEQUENTIAL;
    } else if (selection_string == "sequential_ascending") {
        return M_SEQUENTIAL_ASCENDING;
    } else if (selection_string == "sequential_descending") {
        return M_SEQUENTIAL_DESCENDING;
    } else if (selection_string == "random_with_replacement") {
        return M_RANDOM_WITH_REPLACEMENT;
    } else if (selection_string == "random_without_replacement") {
        return M_RANDOM_WOR;
    } else {
        throw SimpleException(M_PARADIGM_MESSAGE_DOMAIN, "invalid value for parameter \"selection\"", s);
    }
}


template<>
SampleType ParameterValue::convert(const std::string &s, ComponentRegistryPtr reg) {
    std::string sampling_method_string(boost::algorithm::to_lower_copy(s));
    
    if (sampling_method_string == "cycles") {
        return M_CYCLES;
    } else if (sampling_method_string == "samples") {
        return M_SAMPLES;
    } else {
        throw SimpleException(M_PARADIGM_MESSAGE_DOMAIN, "invalid value for parameter \"sampling_method\"", s);
    }
}


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


weak_ptr<State> ListState::next() {
    
	if(!selection){
		throw SimpleException("Attempt to draw from an invalid selection object");
	}
    
	if (hasMoreChildrenToRun()) {
		//mprintf("I am %s, and I have more children to run!", name);
		int index;
		
		try{
			index = selection->draw();
		} catch(std::exception& e){
			index = -1; // just a bandaid for now
		}
		
		shared_ptr<State> thestate;
        
		if ((index < 0) || !(thestate = getList()[index])) {
			mwarning(M_PARADIGM_MESSAGE_DOMAIN,
                     "List state returned invalid state at index %d",
                     index);
            return State::next();
		}
		
		shared_ptr<State> thestate_parent(thestate->getParent()); 
        
		if (thestate_parent.get() != this) {
			// this ensures that we find our way back, 
			// even if something is screwed up
			thestate->setParent(component_shared_from_this<State>());
			thestate->updateHierarchy(); // TODO: might want to do this differently
		}
		
		thestate->updateCurrentScopedVariableContext();
		return thestate;		
	} else {
        return State::next();
	}
}


void ListState::reset() {
    ContainerState::reset();
	if(selection != NULL) {
		selection->reset();
	} else {
		mwarning(M_PARSER_MESSAGE_DOMAIN,"attempt to reset NULL selection object");
	}
}
	
	
void ListState::finalize(std::map<std::string, std::string> parameters, ComponentRegistry *reg) {
    long N = nsamples;
	
	if (sampling_method == M_CYCLES) {
		N *= getNItems();
	}
	
	Selection *selection = NULL;	
	switch(selection_type){
        case M_SEQUENTIAL:
        case M_SEQUENTIAL_ASCENDING:
            selection = new SequentialSelection(N);
            break;
	
		case M_SEQUENTIAL_DESCENDING:
			selection = new SequentialSelection(N, false);
			break;
			
		case M_RANDOM_WITH_REPLACEMENT:
			selection = new RandomWithReplacementSelection(N);
			break;
            
        case M_RANDOM_WOR:
            selection = new RandomWORSelection(N);
            break;
	}
	
	shared_ptr<Selection> sel_ptr(selection);
	attachSelection(sel_ptr);
}


END_NAMESPACE_MW


























