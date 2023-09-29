#include "Experiment.h"

#include "ComponentRegistry.h"
#include "Event.h"
#include "EventBuffer.h"
#include "ExpressionVariable.h"
#include "OpenGLContextManager.h"
#include "StimulusDisplay.h"
#include "SystemEventFactory.h"


BEGIN_NAMESPACE_MW


shared_ptr<Experiment> GlobalCurrentExperiment; // a kludge for now


Experiment::Experiment(shared_ptr<VariableRegistry> var_reg)
: ContainerState() {
    variable_registry = var_reg;
	
	//current_protocol = NULL;
	n_protocols = 0;
	
    experimentName = "";
    
    shouldCreateDefaultStimulusDisplay = false;
	
}


Experiment::~Experiment() { }


void Experiment::createVariableContexts() {
    for (auto &var : getVariables()) {
        local_variable_context->set(var->getContextIndex(), var->getProperties().getDefaultValue());
    }
}


boost::shared_ptr<StimulusDisplay> Experiment::getDefaultStimulusDisplay() {
    lock_guard lock(displaysMutex);
    
    if (!defaultDisplay) {
        if (!shouldCreateDefaultStimulusDisplay) {
            throw SimpleException(M_DISPLAY_MESSAGE_DOMAIN,
                                  "The default stimulus display is not available.  You must explicitly specify the "
                                  "display to use.");
        }
        auto mainScreenInfo = ComponentRegistry::getSharedRegistry()->getVariable(MAIN_SCREEN_INFO_TAGNAME);
        const auto config = StimulusDisplay::getDisplayConfiguration(mainScreenInfo->getValue());
        defaultDisplay = StimulusDisplay::prepareStimulusDisplay(config);
    }
    
    return defaultDisplay;
}


void Experiment::addStimulusDisplay(const boost::shared_ptr<StimulusDisplay> &display) {
    lock_guard lock(displaysMutex);
    nonDefaultDisplays.emplace_back(display);
}


void Experiment::clearStimulusDisplays() {
    lock_guard lock(displaysMutex);
    if (defaultDisplay) {
        defaultDisplay->clearDisplay();
    } else {
        for (const auto &display : nonDefaultDisplays) {
            display->clearDisplay();
        }
    }
}


void Experiment::setCurrentProtocol(unsigned int protocol_number) {
    if(protocol_number >= getList().size()) {
		merror(M_PARADIGM_MESSAGE_DOMAIN,
			   "Attempt to access invalid protocol number");
    }
	
    current_protocol = boost::dynamic_pointer_cast<mw::Protocol, State>(getList()[protocol_number]);
	
	if(current_protocol != 0) {
		global_outgoing_event_buffer->putEvent(SystemEventFactory::protocolPackage());
	}
}

void Experiment::setCurrentProtocol(std::string protName) {
    mprintf("Setting protocol to %s", protName.c_str());
	if(protName.size() == 0) { return; }
    for(unsigned int i = 0; i < getList().size(); i++) {
        std::string comp = (getList()[i])->getName();
		
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


void Experiment::action(){
	variable_registry->announceAll();
	
	if(current_protocol.use_count() == 0 && getList().size()){
		// TODO: this line is bad
		current_protocol =  boost::dynamic_pointer_cast<mw::Protocol, State>(getList()[0]); // TODO: remove up-cast;
	}
	
	ContainerState::action();
}


weak_ptr<State> Experiment::next() {
    if (!accessed) {
        accessed = true;
        return current_protocol;
    }
    return getEndState();
}


void Experiment::reset(){
    ContainerState::reset();
	
    clearStimulusDisplays();
	
	//variable_registry->reset();
	//	delete variable_registry;
	//	variable_registry = new VariableRegistry();
	//	initializeStandardVariables(variable_registry);
	//	global_variable_registry = variable_registry;
	
	
	
	// issue an event that the experiment has stopped
	global_outgoing_event_buffer->putEvent(SystemEventFactory::currentExperimentState());
}


shared_ptr<VariableRegistry> Experiment::getVariableRegistry() {
    return variable_registry;
}


void Experiment::setExperimentName(std::string n){
	experimentName = n;
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

std::string Experiment::getExperimentDirectory() {
    namespace bf = boost::filesystem;
    return bf::path(experimentPath).filename().string();
}


boost::shared_ptr<Component> ExperimentFactory::createObject(std::map<std::string, std::string> parameters,
                                                             ComponentRegistry *reg)
{
    auto experiment = boost::make_shared<Experiment>(global_variable_registry);
    
    const auto &experimentName = parameters["tag"];
    if (!experimentName.empty()) {
        experiment->setExperimentName(experimentName);
    }
    
    experiment->setWorkingPath(parameters["working_path"]);
    
    const auto &shouldCreateDefaultStimulusDisplay = parameters["should_create_default_stimulus_display"];
    if (!shouldCreateDefaultStimulusDisplay.empty()) {
        experiment->setShouldCreateDefaultStimulusDisplay(reg->getBoolean(shouldCreateDefaultStimulusDisplay));
    }
    
    GlobalCurrentExperiment = experiment;
    
    return experiment;
}


END_NAMESPACE_MW
