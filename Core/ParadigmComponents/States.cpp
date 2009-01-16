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

#include <string.h>
#include "ComponentRegistry_new.h"

#include "SequentialSelection.h"
#include "RandomWORSelection.h"
#include "RandomWithReplacementSelection.h"

using namespace mw;

///////////////////////////////////////////////////////////////////
// -> Required Parameters
//
// Although parameters are normally completely user defined,
// the following parameters must be declared for the collection
// of state objects to function properly.  They are only declared
// here.  They must still be defined and registered by the user
// in UserData.h
//
///////////////////////////////////////////////////////////////////

//mVariabletaskMode_edit;
//mVariableSAMPLING_ON;
//mVariableIS_RUNNING;


State::State() : ScopedVariableEnvironment() {
	experiment = weak_ptr<Experiment>(GlobalCurrentExperiment);
	parent = weak_ptr<State>();
    name = "";
    description = "";

	requestVariableContext();

	interruptible = true;

}


void State::requestVariableContext(){
												
	if(!experiment.expired()) {
		shared_ptr<Experiment> experiment_shared(experiment);
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


//mState::State(Experiment *exp) : ScopedVariableEnvironment() {
//    experiment = exp;
//	parent = weak_ptr<State>();
//    name = "mState";
//    description = "";
//	if(experiment) {
//		local_variable_context = experiment->createNewContext();
//		setScopedVariableEnvironment(experiment);
//	} else if(GlobalCurrentExperiment){
//		local_variable_context = GlobalCurrentExperiment->createNewContext();
//		setScopedVariableEnvironment(GlobalCurrentExperiment);
//	} else {
//		merror(M_SYSTEM_MESSAGE_DOMAIN,
//			   "Attempt to create a state object without a valid variable environment in place (2)");
//	}
//
//	setCurrentContext(local_variable_context);
//	interruptible = true;
//	
//}
	
//mState::State(weak_ptr<State> newparent) : ScopedVariableEnvironment()  {
//    parent = newparent;
//    name = "mState";
//    description ="";
//	
//	interruptible = true;
//	
//    if(parent) {
//        experiment = parent->getExperiment();
//    } else {
//        experiment = NULL;
//    }
//    
//	if(experiment == NULL){
//		experiment = GlobalCurrentExperiment;
//	}
//	
//	// TODO: still a problem here
//	if(experiment != NULL) {
//		local_variable_context = experiment->createNewContext();
//		setScopedVariableEnvironment(experiment);
//	} else {
//		merror(M_SYSTEM_MESSAGE_DOMAIN,
//			   "Attempt to create a state object without a valid variable environment in place (3)");
//		local_variable_context = shared_ptr<ScopedVariableContext>(new ScopedVariableContext(NULL));
//	}
//	
//	setCurrentContext(local_variable_context);
//	
//    if(parent != NULL && parent->getLocalScopedVariableContext() != NULL){
////        local_variable_context = new ScopedVariableContext(parent->getLocalScopedVariableContext());
//		local_variable_context->inheritFrom(parent->getLocalScopedVariableContext());
//	}// else {
////	    local_variable_context = new ScopedVariableContext();  // DEAL WITH THIS EVENTUALLY
// //   }
//	
//}
	
State::~State() { }

void State::action() {
    currentState->setValue(getCompactID());
	//currentState->setValue(tag);
	
	if(!experiment.expired()){
		shared_ptr<Experiment> experiment_shared(experiment);
		experiment_shared->announceLocalVariables();
	}
	
	updateCurrentScopedVariableContext();
}

void  State::update() { } 	

weak_ptr<State> State::next() {
    return weak_ptr<State>();
}

void State::setParent(weak_ptr<State> newparent) {
    parent = newparent;
    
    if(!newparent.expired()) {
		shared_ptr<State> newparent_locked = shared_ptr<State>(newparent);
        setExperiment(newparent_locked->getExperiment());
        setScopedVariableEnvironment(newparent_locked->getScopedVariableEnvironment());
    } else {
        mprintf("Setting experiment and localVariableEnvironment fields to NULL");
        setExperiment(weak_ptr<Experiment>());
        setScopedVariableEnvironment(weak_ptr<ScopedVariableEnvironment>());
    }
                
    if(local_variable_context == NULL) {
		merror(M_SYSTEM_MESSAGE_DOMAIN,
			  "Invalid variable context in state object");
        local_variable_context = shared_ptr<ScopedVariableContext>(new ScopedVariableContext(NULL));
    }
	
	shared_ptr<State> parent_locked(parent);
    if(parent_locked != NULL && parent_locked->getLocalScopedVariableContext() != NULL) {
        local_variable_context->inheritFrom(parent_locked->getLocalScopedVariableContext());
    } else {
		merror(M_SYSTEM_MESSAGE_DOMAIN,
			   "Invalid parent or parent variable context");
		//throw("I don't know what will happen here yet...");
		//local_variable_context = new ScopedVariableContext();// DEAL WITH THIS EVENTUALLY
	}
	
	setCurrentContext(local_variable_context);
}	
	
void State::updateHierarchy() {
	if(parent.expired()) {
        // do nothing
    } else {
        setParent(parent);  // refresh LocalScopedVariableContext, experiment, etc.
    }
}

void State::updateCurrentScopedVariableContext() {
	if(!environment.expired()){
		shared_ptr<ScopedVariableEnvironment> environment_shared(environment);
		environment_shared->setCurrentContext(local_variable_context);
		
		if(!parent.expired()) {
			shared_ptr<State> parent_shared(parent);
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

void State::reset() { }

//void  State::announceIdentity() {
//	Data nameString(name);
//	announce(nameString);
//}
//

weak_ptr<State> State::getParent() {
    return parent;
}

weak_ptr<Experiment> State::getExperiment() {
    return experiment;
}
        
void State::setExperiment(weak_ptr<Experiment> _experiment) {
    experiment = _experiment;
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

// semi shallow copy
State *State::getStateInstance(){
	// clone this object
	// this depends on an appropriate implementation
	// of clone in inherited versions of this class
	State *new_state = (State *)scopedClone();
	
	// "inherit" the local variable context from this object
	//new_state->inheritLocalScopedVariableContext(getLocalScopedVariableContext());
	
	return new_state;
}

shared_ptr<mw::Component> State::createInstanceObject(){
//void *State::scopedClone(){
	
	State *new_state = new State();
	new_state->setParent(parent);
	//new_state->setLocalScopedVariableContext(new ScopedVariableContext());
	//new_state->setLocalScopedVariableContext(getLocalScopedVariableContext());
	new_state->setExperiment(getExperiment());
	new_state->setScopedVariableEnvironment(getScopedVariableEnvironment());
	new_state->setDescription(getDescription());
	new_state->setName(getName());
	
	shared_ptr<mw::Component> clone_ptr(new_state);
	return clone_ptr;
}

// TODO: deprecated
void *State::scopedClone(){
	
	State *new_state = new State();
	new_state->setParent(parent);
	//new_state->setLocalScopedVariableContext(new ScopedVariableContext());
	//new_state->setLocalScopedVariableContext(getLocalScopedVariableContext());
	new_state->setExperiment(getExperiment());
	new_state->setScopedVariableEnvironment(getScopedVariableEnvironment());
	new_state->setDescription(getDescription());
	new_state->setName(getName());
	
	return new_state;
}



void State::setName(std::string n) {  
	name = n;
}

void State::setName(const char *n){
	name = n;
}

std::string State::getName() {
    return name;
}

void State::setDescription(std::string d) {
	description = d;
}

std::string State::getDescription() {
    return description;
}


/*shared_ptr<mw::Component> State::createInstanceObject(){
	mw::Component *clone = (mw::Component *)scopedClone();
	shared_ptr<mw::Component> the_alias((mw::Component *)clone);
	return the_alias;
}*/
        

StateReference::StateReference(State *ref){
	state = ref;
}
		
void StateReference::action(){
	weak_ptr<State> old_parent = state->getParent();
	state->setParent(parent);
	state->action();
	state->setParent(old_parent);
}
weak_ptr<State> StateReference::next(){
	weak_ptr<State> old_parent = state->getParent();
	state->setParent(parent);
	weak_ptr<State> return_state = state->next();
	state->setParent(old_parent);
	return return_state;
}


void StateReference::update(){
	state->update();
}

//mContainerState::ContainerState(Experiment *exp) : 
//	 State(exp),
//	 list(new vector< shared_ptr<State> >){
//	 
//	accessed = false;
//	setName("mContainerState");
//}

ContainerState::ContainerState() : 
	State(),
	list(new vector< shared_ptr<State> >){
	
	accessed = false;
	setName("mContainerState");
}

/*ContainerState::ContainerState(State *parent) : State(parent){
	list = new ExpandableList<State>();
	accessed = false;
}*/

ContainerState::~ContainerState() {
/*	if(!isAClone()){
		delete(list);
	}	*/
}

/*void ContainerState::addState(shared_ptr<State> newstate) {

	if(newstate == NULL) {
		return;
		//throw("Attempt to add a null state to a list state");
	}
	
	// add the new object to the expandable list
	list->addReference(newstate);
	newstate->setParent(this);
	
	// recurse throughout the hierarchy to update parents, 
	// LocalScopedVariableContext, & experiment
	////newstate->updateHierarchy(); 
	
}
	
void ContainerState::addState(int index, shared_ptr<State> newstate) { 
	list->addReference(index, newstate);
	newstate->setParent(this);
	newstate->updateHierarchy(); 
}*/


shared_ptr<mw::Component> ContainerState::createInstanceObject(){
//void *ContainerState::scopedClone(){

	ContainerState *new_state = new ContainerState();
	new_state->setExperiment(getExperiment());
	new_state->setScopedVariableEnvironment(getScopedVariableEnvironment());
	new_state->setDescription(getDescription());
	new_state->setName(getName());
	

	new_state->setList(list);
	
	new_state->setIsAClone(true);	// in case we care for memory-freeing purposes
	shared_ptr<mw::Component> clone_ptr(new_state);
	return clone_ptr;
}

//
//mListState::ListState(State *newparent): ContainerState(newparent) {
//	has_more_children_to_run = true;
//	setName((char *)"base ListState");
//}

ListState::ListState() : ContainerState() {

	has_more_children_to_run = true;
	setName((char *)"base ListState");
}

ListState::~ListState() {
	
	
	//delete(local_variable_context);
}


shared_ptr<mw::Component> ListState::createInstanceObject(){
//void *ListState::scopedClone(){

	ListState *new_state = new ListState();
	new_state->setParent(parent);
	//new_state->setLocalScopedVariableContext(new ScopedVariableContext());
	new_state->setExperiment(getExperiment());
	new_state->setScopedVariableEnvironment(getScopedVariableEnvironment());
	new_state->setDescription(getDescription());
	new_state->setName(getName());
	
	//new_state->setN(getN());
	//new_state->setSamplingMethod(getSamplingMethod());
	

	new_state->setList(list);
	
	shared_ptr<Selection> sel = getSelectionClone();
	if(sel != shared_ptr<Selection>()){
		new_state->attachSelection(sel);
		//new_state->setSelection((Selection *)(sel->clone()));
	}
	
	// TODO: was this safe to remove?
	/*if(new_state->getSelection() != NULL){
		(new_state->getSelection())->setNItems(list->getNElements());
	}*/
	
	new_state->setIsAClone(true);	// in case we care for memory-freeing purposes
	shared_ptr<mw::Component> clone_ptr(new_state);
	return clone_ptr;
}



void State::setParameters(std::map<std::string, std::string> parameters,
											mwComponentRegistry *reg){
	if(parameters.find("interruptible") != parameters.end()) {
		try {
			bool _interruptible = reg->getBoolean(parameters.find("interruptible")->second);
			this->setInterruptible(_interruptible);
		} catch(bad_lexical_cast &) {
			throw InvalidReferenceException(parameters.find("reference_id")->second, "interruptible", parameters.find("interruptible")->second);
		}
	} else {
		this->setInterruptible(true);
	}
	
	if(parameters.find("tag") != parameters.end()) {
		this->setName(parameters.find("tag")->second);
	} else {
//		this->setName(tag);				
	}
	
	if(parameters.find("description") != parameters.end()) {
		this->setDescription(parameters.find("description")->second);
	}
}
	
	
void ListState::finalize(std::map<std::string, std::string> parameters,
												mwComponentRegistry *reg){
		
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
	if(parent.expired()) {
		// do nothing
	} else {
		// update various settings with respect to hierarchy
		setParent(parent); 
	}
	
	for(unsigned int i = 0; i < list->size(); i++) {
		// recurse down the hierarchy
		shared_ptr<State> this_element = list->operator[](i);
		
		if(this_element == NULL){
			merror(M_PARADIGM_MESSAGE_DOMAIN,
					"Null child in container component.  This will cause problems");
		
			continue;
		}
		
		this_element->updateHierarchy(); 
	}
}


int ListState::getNChildren() {
	return list->size();
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
			if(!parent.expired()){
				update();
				return parent;
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
			if(!parent.expired()){
				update();
				return parent;
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
			thestate->setParent(getSelfPtr<State>());
			updateHierarchy(); // TODO: might want to do this differently
		}
		
		thestate->updateCurrentScopedVariableContext();
		return thestate;		
	} else {
		//mprintf("Returning Parent!!!");	
		// wait a bit so that I can see what's going on...
		//tick(10000); 
		// update my parent to update done tables, etc.
		shared_ptr<State> parent_shared(parent);
		parent_shared->update(); 
		parent_shared->updateCurrentScopedVariableContext();
		reset();
		return parent;
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

int ListState::getNItems() { return getNChildren(); }


//template <class U, class E>
//void setInfo(E, U){};

shared_ptr< vector< shared_ptr<State> > > ContainerState::getList() { return list; }
void ContainerState::setList(shared_ptr< vector< shared_ptr<State> > > newlist){  list = newlist; }
