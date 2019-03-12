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
	
    //stimulus_display = NULL;
	
	// This fields will eventually be self-referential (set during "finalize")
	current_state = weak_ptr<State>();
	
    experimentName = "";
	setName(experimentName);
	
}


Experiment::~Experiment() { }


void Experiment::createVariableContexts(){
	
	if(variable_registry == NULL){
		// TODO: warn
		return;
	}
	
    setLocalScopedVariableContext(createNewDefaultContext());
	
}


shared_ptr<StimulusDisplay> Experiment::getStimulusDisplay() {
    std::call_once(stimulusDisplayCreated, [this]() {
        prepareStimulusDisplay();
    });
    return stimulus_display;
}


void Experiment::setCurrentProtocol(unsigned int protocol_number) {
    if(protocol_number >= getList().size()) {
		merror(M_PARADIGM_MESSAGE_DOMAIN,
			   "Attempt to access invalid protocol number");
    }
	
    current_protocol = boost::dynamic_pointer_cast<mw::Protocol, State>(getList()[protocol_number]);
	
	// TODO: is this implicit cast kosher?
	current_state = weak_ptr<State>(current_protocol);
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
	
	if(current_protocol.use_count() == 0 && getList().size()){
		// TODO: this line is bad
		current_protocol =  boost::dynamic_pointer_cast<mw::Protocol, State>(getList()[0]); // TODO: remove up-cast;
	}
	
	ContainerState::action();
}


weak_ptr<State> Experiment::next() {
    if (!accessed) {
        accessed = true;
        current_protocol->updateCurrentScopedVariableContext();
        return current_protocol;
    } else {
        current_state = weak_ptr<State>();
        *state_system_mode = STOPPING;
        return current_state;
    }
}


void Experiment::reset(){
    ContainerState::reset();
	
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
	//	global_variable_registry = variable_registry;
	
	
	
	// issue an event that the experiment has stopped
	global_outgoing_event_buffer->putEvent(SystemEventFactory::currentExperimentState());
}


shared_ptr<VariableRegistry> Experiment::getVariableRegistry() {
    return variable_registry;
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

std::string Experiment::getExperimentDirectory() {
    namespace bf = boost::filesystem;
    return bf::path(experimentPath).filename().string();
}


void Experiment::prepareStimulusDisplay() {
    auto opengl_context_manager = OpenGLContextManager::instance(false);
    if (!opengl_context_manager) {
        opengl_context_manager = OpenGLContextManager::createPlatformOpenGLContextManager();
        OpenGLContextManager::registerInstance(opengl_context_manager);
    }
    
    bool always_display_mirror_window = false;
    int display_to_use = 0;
    bool announce_individual_stimuli = true;
    bool use_color_management = true;
    bool make_window_opaque = true;
    
    if (auto main_screen_info = ComponentRegistry::getSharedRegistry()->getVariable(MAIN_SCREEN_INFO_TAGNAME)) {
        Datum val = *(main_screen_info);
        
        if (val.hasKey(M_DISPLAY_TO_USE_KEY)) {
            display_to_use = (int)val.getElement(M_DISPLAY_TO_USE_KEY);
        }
        
        if (val.hasKey(M_ALWAYS_DISPLAY_MIRROR_WINDOW_KEY)) {
            always_display_mirror_window = (bool)val.getElement(M_ALWAYS_DISPLAY_MIRROR_WINDOW_KEY);
        }
        
        if (val.hasKey(M_ANNOUNCE_INDIVIDUAL_STIMULI_KEY)) {
            announce_individual_stimuli = (bool)val.getElement(M_ANNOUNCE_INDIVIDUAL_STIMULI_KEY);
        }
        
        if (val.hasKey(M_USE_COLOR_MANAGEMENT_KEY)) {
            use_color_management = (bool)val.getElement(M_USE_COLOR_MANAGEMENT_KEY);
        }
        
        if (val.hasKey(M_MAKE_WINDOW_OPAQUE_KEY)) {
            make_window_opaque = (bool)val.getElement(M_MAKE_WINDOW_OPAQUE_KEY);
        }
    }
    
    stimulus_display = StimulusDisplay::createPlatformStimulusDisplay(announce_individual_stimuli,
                                                                      use_color_management);
    
    if (display_to_use >= 0 && (opengl_context_manager->getNumDisplays() > 1 || display_to_use == 0)) {
        if (display_to_use >= opengl_context_manager->getNumDisplays()) {
            merror(M_SERVER_MESSAGE_DOMAIN,
                   "Requested display index (%d) is greater than the number of displays (%d).  "
                   "Using default display.",
                   display_to_use,
                   opengl_context_manager->getNumDisplays());
            display_to_use = 1;
        }
        
        auto new_context = opengl_context_manager->newFullscreenContext(display_to_use, make_window_opaque);
        stimulus_display->addContext(new_context);
        
        if (always_display_mirror_window) {
            auto auxilliary_context = opengl_context_manager->newMirrorContext();
            stimulus_display->addContext(auxilliary_context);
        }
    } else {
        auto new_context = opengl_context_manager->newMirrorContext();
        stimulus_display->addContext(new_context);
    }
    
    stimulus_display->clearDisplay();
}


END_NAMESPACE_MW




















