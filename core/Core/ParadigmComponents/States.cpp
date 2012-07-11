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

#include "SequentialSelection.h"
#include "RandomWORSelection.h"
#include "RandomWithReplacementSelection.h"


BEGIN_NAMESPACE_MW


State::State() : ScopedVariableEnvironment() {
	experiment = weak_ptr<Experiment>(GlobalCurrentExperiment);
    description = "";

	requestVariableContext();

	interruptible = true;

}


void State::requestVariableContext(){
												
    shared_ptr<Experiment> experiment_shared = experiment.lock();
	if(experiment_shared) {
		local_variable_context = experiment_shared->createNewContext();
		weak_ptr<ScopedVariableEnvironment> env(dynamic_pointer_cast<ScopedVariableEnvironment, Experiment>(GlobalCurrentExperiment)); 
		setScopedVariableEnvironment(env);
	} else if(GlobalCurrentExperiment != NULL){
		experiment = weak_ptr<Experiment>(GlobalCurrentExperiment);
		local_variable_context = GlobalCurrentExperiment->createNewContext();
		weak_ptr<ScopedVariableEnvironment> env(dynamic_pointer_cast<ScopedVariableEnvironment, Experiment>(GlobalCurrentExperiment)); 
		setScopedVariableEnvironment(env);
	} else {
		//merror(M_PARSER_MESSAGE_DOMAIN,
		//		"Unable to set scoped variable environment");
	}

	setCurrentContext(local_variable_context);
												
}


void State::action() {
    currentState->setValue(getCompactID());
	
    shared_ptr<Experiment> experiment_shared(getExperiment());
	if (experiment_shared) {
		experiment_shared->announceLocalVariables();
	}
	
	updateCurrentScopedVariableContext();
}


weak_ptr<State> State::next() {
    return weak_ptr<State>();
}


void State::updateHierarchy() {
    shared_ptr<State> sharedParent = getParent();
	if (!sharedParent) {
        return;
    }
    
    setExperiment(sharedParent->getExperiment());
    setScopedVariableEnvironment(sharedParent->getScopedVariableEnvironment());
    
    if (local_variable_context == NULL) {
		merror(M_SYSTEM_MESSAGE_DOMAIN,
               "Invalid variable context in state object");
        local_variable_context = shared_ptr<ScopedVariableContext>(new ScopedVariableContext(NULL));
    }
	
    if (sharedParent->getLocalScopedVariableContext()) {
        local_variable_context->inheritFrom(sharedParent->getLocalScopedVariableContext());
    } else {
		merror(M_SYSTEM_MESSAGE_DOMAIN, "Invalid parent variable context");
		//throw("I don't know what will happen here yet...");
		//local_variable_context = new ScopedVariableContext();// DEAL WITH THIS EVENTUALLY
	}
	
	setCurrentContext(local_variable_context);
}


void State::updateCurrentScopedVariableContext() {
    shared_ptr<ScopedVariableEnvironment> environment_shared = environment.lock();
	if(environment_shared){
		environment_shared->setCurrentContext(local_variable_context);
		
        shared_ptr<State> parent_shared(getParent());
		if (parent_shared) {
			shared_ptr<ScopedVariableContext> parentContext = parent_shared->getLocalScopedVariableContext();
			if(parentContext) {
				int numScopedVars = local_variable_context->getNFields();
				for (int i=0; i<numScopedVars; ++i) {
					if(local_variable_context->getTransparency(i) == M_TRANSPARENT) {
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


weak_ptr<ScopedVariableEnvironment> State::getScopedVariableEnvironment() {
    return environment;
}

void State::setScopedVariableEnvironment(weak_ptr<ScopedVariableEnvironment> _env) {
    environment = _env;
}

void State::setLocalScopedVariableContext(shared_ptr<ScopedVariableContext> c) {  
	local_variable_context = c;
	setCurrentContext(local_variable_context); 
}

shared_ptr<ScopedVariableContext> State::getLocalScopedVariableContext() {
    return local_variable_context;
}

shared_ptr<mw::Component> State::createInstanceObject(){
    return clone<State>();
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
        

ContainerState::ContainerState() : 
	State(),
	list(new vector< shared_ptr<State> >){
	
	accessed = false;
	setName("mContainerState");
}


shared_ptr<mw::Component> ContainerState::createInstanceObject(){
    return clone<ContainerState>();
}


ListState::ListState() : ContainerState() {

	has_more_children_to_run = true;
	setName((char *)"base ListState");
}


shared_ptr<mw::Component> ListState::createInstanceObject(){
    return clone<ListState>();
}


void State::setParameters(std::map<std::string, std::string> parameters,
											ComponentRegistry *reg){
	if(parameters.find("interruptible") != parameters.end()) {
		try {
			bool _interruptible = reg->getBoolean(parameters.find("interruptible")->second);
			this->setInterruptible(_interruptible);
		} catch(bad_lexical_cast &) {
			throw InvalidReferenceException(parameters["reference_id"], "interruptible", parameters.find("interruptible")->second);
		}
	} else {
		this->setInterruptible(true);
	}
	
	if(parameters.find("tag") != parameters.end()) {
		this->setTag(parameters.find("tag")->second);
	}
	
	if(parameters.find("description") != parameters.end()) {
		this->setDescription(parameters.find("description")->second);
	}
}
	
	
void ListState::finalize(std::map<std::string, std::string> parameters,
												ComponentRegistry *reg){
		
	std::string selection_string, nsamples_string, sampling_method_string;
	int nsamples;
	SelectionType selection_type;
	SampleType sampling_method;
	
	if(parameters.find("selection") != parameters.end()) {
		selection_string = parameters.find("selection")->second;
		
		if(selection_string == "sequential_ascending"){
			selection_type = M_SEQUENTIAL_ASCENDING;
		} else if(selection_string == "sequential"){
			selection_type = M_SEQUENTIAL_ASCENDING;
		} else if(selection_string == "sequential_descending"){
			selection_type = M_SEQUENTIAL_DESCENDING;
		} else if (selection_string == "random_with_replacement"){
			selection_type = M_RANDOM_WITH_REPLACEMENT;
		} else if(selection_string == "random_without_replacement"){
			selection_type = M_RANDOM_WOR;
		} else {
			selection_type = M_SEQUENTIAL_ASCENDING;
		}
	}

	if(parameters.find("nsamples") != parameters.end()) {
		nsamples_string = parameters.find("nsamples")->second;
		nsamples = boost::lexical_cast<long>(nsamples_string);
	}
	
	if(parameters.find("sampling_method") != parameters.end()) {
		sampling_method_string = parameters.find("sampling_method")->second;
		
		if(sampling_method_string == "samples"){
			sampling_method = M_SAMPLES;
		} else if(sampling_method_string == "cycles"){
			sampling_method = M_CYCLES;
		} else {
			sampling_method = M_CYCLES;
		}
	}
	
	long N;
	
	if(sampling_method == M_SAMPLES){
		N = nsamples;
	} else {
		N = nsamples * getNChildren();
	}
	
	Selection *selection = NULL;	
	switch(selection_type){
	
		case M_SEQUENTIAL_DESCENDING:
			selection = new SequentialSelection(N, false);
			break;
			
		case M_RANDOM_WOR:
			selection = new RandomWORSelection(N);
			break;
			
		case M_RANDOM_WITH_REPLACEMENT:
			selection = new RandomWithReplacementSelection(N);
			break;
			
		case M_SEQUENTIAL_ASCENDING:
		default:
			selection = new SequentialSelection(N);
			break;
	}
	
	shared_ptr<Selection> sel_ptr(selection);
	attachSelection(sel_ptr);

}

void ContainerState::updateHierarchy() {
    State::updateHierarchy();
    
    shared_ptr<State> self_ptr = component_shared_from_this<State>();
	
	for(unsigned int i = 0; i < list->size(); i++) {
		// recurse down the hierarchy
		shared_ptr<State> this_element = (*list)[i];
		
		if(this_element == NULL){
			merror(M_PARADIGM_MESSAGE_DOMAIN,
					"Null child in container component.  This will cause problems");
		
			continue;
		}
		
        this_element->setParent(self_ptr);
		this_element->updateHierarchy(); 
	}
}


weak_ptr<State> ListState::next() {

	if(!selection){
		throw SimpleException("Attempt to draw from an invalid selection object");
	}

	if(has_more_children_to_run) {
		//mprintf("I am %s, and I have more children to run!", name);
		int index;
		
		try{
			index = selection->draw();
		
		} catch(std::exception& e){
			index = -1; // just a bandaid for now
		}
		
		if(index < 0){
			mwarning(M_PARADIGM_MESSAGE_DOMAIN,
				"List state returned invalid state at index %d (1)",
				index);
            shared_ptr<State> sharedParent = getParent();
			if (sharedParent) {
				update();
				return sharedParent;
			} else {
				
				shared_ptr <Clock> clock = Clock::instance();
				clock->sleepMS(10000);
				return weak_ptr<State>();
			}
		}
		//mprintf("State System: Fetching sub-state #%d", index);
		shared_ptr<State> thestate = (list->operator[](index));
		if(thestate == NULL){		
			mwarning(M_PARADIGM_MESSAGE_DOMAIN,
				"List state returned invalid state at index %d (2)",
				index);
            shared_ptr<State> sharedParent = getParent();
			if (sharedParent) {
				update();
				return sharedParent;
			} else {
				shared_ptr <Clock> clock = Clock::instance();
				clock->sleepMS(10000);
				return weak_ptr<State>();
			}
		}
		
		shared_ptr<State> thestate_parent(thestate->getParent()); 

		if(thestate != NULL && thestate_parent.get() != this) {
			// this ensures that we find our way back, 
			// even if something is screwed up
			thestate->setParent(component_shared_from_this<State>());
			updateHierarchy(); // TODO: might want to do this differently
		}
		
		thestate->updateCurrentScopedVariableContext();
		return thestate;		
	} else {
		//mprintf("Returning Parent!!!");	
		// wait a bit so that I can see what's going on...
		//tick(10000); 
		// update my parent to update done tables, etc.
		shared_ptr<State> parent_shared(getParent());
		parent_shared->update(); 
		parent_shared->updateCurrentScopedVariableContext();
		reset();
		return parent_shared;
	}
}


void ListState::update(){
	//mprintf("updating list state");
	if(selection->isFinished()) {
		has_more_children_to_run = false;
	}
}


void ListState::reset() {
	has_more_children_to_run = true;
	for(int i = 0; i < getNChildren(); i++) {
		// call recursively down the experiment hierarchy
		(*list)[i]->reset();  
	}
	if(selection != NULL) {
		selection->reset();
	} else {
		mwarning(M_PARSER_MESSAGE_DOMAIN,"attempt to reset NULL selection object");
	}
}


END_NAMESPACE_MW
