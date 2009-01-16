#include "Experiment.h"
#include "EventBuffer.h"
#include "Event.h"
#include "EventFactory.h"
#include "ConstantVariable.h"
#include "ExpressionVariable.h"
//#include "UserData.h"
using namespace mw;

namespace mw {
	shared_ptr<Experiment> GlobalCurrentExperiment; // a kludge for now
}

Experiment::Experiment(shared_ptr<VariableRegistry> var_reg)
: ContainerState() {
    variable_registry = var_reg;
	
	//current_protocol = NULL;
	n_protocols = 0;
	
    //stimulus_display = NULL;
	
	// These fields will eventually be self-referential (set during "finalize")
	current_state = weak_ptr<State>();
	experiment = weak_ptr<Experiment>();
	environment = weak_ptr<ScopedVariableEnvironment>();
	
    experimentName = "";
	setName(experimentName);
	
}


Experiment::~Experiment() { }


// The following two because SWIG eats ass
State *Experiment::getStateInstance(){ 
	return (State *)clone(); 
}


void Experiment::createVariableContexts(){
	
	if(variable_registry == NULL){
		// TODO: warn
		return;
	}
	
	local_variable_context = createNewDefaultContext();
	setCurrentContext(local_variable_context);
	//local_variable_context = variable_registry->createLocalVariableContext();
	
}

shared_ptr<StimulusDisplay> Experiment::getStimulusDisplay() {
    return stimulus_display;
}

void Experiment::setStimulusDisplay(shared_ptr<StimulusDisplay> newdisplay) {
    stimulus_display = newdisplay;
}


void Experiment::setCurrentProtocol(unsigned int protocol_number) {
    if(protocol_number < 0 || protocol_number > list->size()) {
		merror(M_PARADIGM_MESSAGE_DOMAIN,
			   "Attempt to access invalid protocol number");
    }
	
    current_protocol = boost::dynamic_pointer_cast<mw::Protocol, State>((*list)[protocol_number]);
	
	// TODO: is this implicit cast kosher?
	current_state = weak_ptr<State>(current_protocol);
	if(current_protocol != 0) {
		GlobalBufferManager->putEvent(EventFactory::protocolPackage());
	}
}

void Experiment::setCurrentProtocol(std::string protName) {
    mprintf("Setting protocol to %s", protName.c_str());
	if(protName.size() == 0) { return; }
    for(unsigned int i = 0; i < list->size(); i++) {
        std::string comp = ((*list)[i])->getName();
		
	    if(comp == protName){
            setCurrentProtocol(i);
            return;
        }
    }
    // if the name was never found print a warning
    mwarning(M_PARADIGM_MESSAGE_DOMAIN,
			 "Attempt to set Experiment to missing protocol -(%s)-", protName.c_str());
}

shared_ptr<mw::Protocol> Experiment::getCurrentProtocol() {
    if(current_protocol == NULL) {
        // DDC: removed, since more than one thread can query this and it is not
        //      thread-safe
        //merror(M_PARADIGM_MESSAGE_DOMAIN,
		//			"Current protocol is NULL, reverting to first protocol");
        //setCurrentProtocol(0);
        //if(current_protocol == NULL) {
		//merror(M_PARADIGM_MESSAGE_DOMAIN,
		//		"Cannot find a valid protocol");
		// TODO: better throw
		throw SimpleException("Cannot find a valid protocol");
        //} else {
		//	return current_protocol;
        //}
    } else {
        return current_protocol;
    }
}



void Experiment::update() {   
    // we got called from the protocol we were running, so we're done for now
    ///setInt(taskMode_edit, IDLE);
    current_state = weak_ptr<State>();
	*task_mode = STOPPING;
}



weak_ptr<State> Experiment::getCurrentState() {
	if(current_state.expired()) {
		return getCurrentProtocol();
	} else {
        return current_state;
	}
}

void Experiment::setCurrentState(weak_ptr<State> newstate) {
    current_state = newstate;
	shared_ptr<State> current_state_shared(current_state);
	current_context = current_state_shared->getLocalScopedVariableContext();
}

void Experiment::action(){
	variable_registry->announceAll();
	
	if(current_protocol.use_count() == 0 && list->size()){
		// TODO: this line is bad
		current_protocol =  boost::dynamic_pointer_cast<mw::Protocol, State>(list->operator[](0)); // TODO: remove up-cast;
	}
	
	ContainerState::action();
}


weak_ptr<State> Experiment::next() {
	current_protocol->updateCurrentScopedVariableContext();
	weak_ptr<State> weak_return(current_protocol);
    return (weak_return);
}


void Experiment::reset(){
	for(unsigned int i = 0; i < list->size(); i++){
		(*list)[i]->reset();
	}
	
	// TODO: is implicit cast kosher?
	weak_ptr<State> state_ptr(current_protocol);
	setCurrentState(state_ptr);
	
	if(stimulus_display != NULL){
		stimulus_display->clearDisplay();
	}
	
	//variable_registry->reset();
	//	delete variable_registry;
	//	variable_registry = new VariableRegistry();
	//	initializeStandardVariables(variable_registry);
	//	GlobalVariableRegistry = variable_registry;
	
	
	
	// issue an event that the experiment has stopped
	GlobalBufferManager->putEvent(EventFactory::currentExperimentState());
}


shared_ptr<VariableRegistry> Experiment::getVariableRegistry() {
    return variable_registry;
}


void setCurrentExperiment(shared_ptr<Experiment> exp) {
	if(exp != NULL) {
		GlobalCurrentExperiment = exp;
	} else {
		merror(M_PARADIGM_MESSAGE_DOMAIN,
			   "Attempt to set current experiment to null");
	}
}

void Experiment::setExperimentName(std::string n){
	experimentName = n;
	setName(experimentName);
}

std::string  Experiment::getExperimentName() {
    return experimentName;
}

void Experiment::setExperimentPath(std::string path) {
	experimentPath = path;
}

std::string Experiment::getExperimentPath() {
	return experimentPath;
}
