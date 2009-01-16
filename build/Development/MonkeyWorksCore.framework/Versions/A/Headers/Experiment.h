#ifndef EXPERIMENT_H
#define EXPERIMENT_H

#include "Stimulus.h"
#include "Utilities.h"
//#include "EyeStatus.h"
#include "KeyTrigger.h"
#include "Stimulus.h"
#include "VariableRegistry.h"
#include "VariableProperties.h"
#include "States.h"
#include "BlockAndProtocol.h"
#include "ScopedVariableEnvironment.h"
#include <string>
#include "LoadingUtilities.h"

/**
 * Experiment.h
 *
 * Description:
 *     The Experiment object encapsulates essentially all of the persistant
 * state of an experiment.  This allows, in principle, multiple experiment
 * objects to exist simultaneously each keeping track of themselves 
 * independently.  Though the experiment object will contain vastly different
 * amounts and kinds of data in different experiments, the code of the 
 * experiment object need not be changed, unless dramatically different 
 * capabilities are required.  Nearly all of the data structures contained 
 * within the experiment object are flexible both in terms of type and 
 * quantity of data they contain.
 *     An experiment object contains a group of protocol objects which belong
 * together, might be run together, and will tend to share variables.  The 
 * protocol object might have reasonably been made the top-level paradigm 
 * object, however, creating one higher level makes it natural to have 
 * variables which apply to all protocols within the group. For instance, we
 * might want the required fixation duration during calibration to be the same
 * as during the actual main experiment itself.  The calibration protocol can
 * be defined in one plugin (and thus be reused in many experiments but use
 * variables defined in the overarching "experiment," thus allow all contained
 * protocols to use a common set of variables (unless explicitly defined
 * otherwise).
 *     As the top-level paradigm component, the experiment object also contains
 * all of the machinery necessary to access variables in a context appropriate
 * manner.  That is, all calls to access a variable go through the experiment
 * object, and the experiment object fetches the appropriate value in the 
 * current context.  This value might be a global one, or one overridden by
 * any stage of the paradigm hierarchy, from the protocol down to the 
 * currently running trial component.
 *     As is, the experiment object also encapsulates stimulus display and 
 * some input-output functionality.  Haven't decided yet whether this is a 
 * good idea.  It would be convenient under this sort of regime for different
 * experiments to use different stimulus-display objects and key triggers, but
 * I'm still struggling a bit with the cleanest OO design concept for this.
 *    One way of conceptualizing the current experiment object is as the root
 * object for everything that a user must specify for a particular experiment 
 * to function (i.e. this doesn't include realtime services, displays, tools,
 * plugin services, etc. etc.).
 * The principal pieces of info contained in the Experiment object are 
 * as follows:
 *
 *    - Current State & Current mw::Protocol: pointers to the currently running 
 *          state and protocol.  State objects (including Protocols, Blocks 
 *          and Trials) are kept on a stack, with the current state being at
 *          the top. Thus, when a mw::Protocol selects the next block to run, it 
 *          is placed on the stack.  The block then determines the next trial
 *          and places it on the stack.  When the trial finishes, it is popped 
 *          off of the stack and the block regains control.  Within this 
 *          structure, an experiment can be nested as deeply as the user 
 *          likes, without increasingly complexity.  Protocols are treated 
 *          specially insofar as their selection is controlled by user input,
 *          and a handle is kept to the current protocol so that the user 
 *          can change protocol-level variables at run-time.
 *
 *    - Experiment-wide "Trial Info" (class ScopedVariableContext):  This object has
 *          info about how the experiment should be run (e.g. stimulus sizes,
 *          indices, ITI etc.).  This object can contain any number of data of
 *          any type. As new objects are placed on the state stack, they 
 *          "inherit" values from the previous object on the the stack. The 
 *          experiment-level TrialInfo object contains the default values 
 *          which are inheritted through the stack until masked by a state 
 *          higher on the stack.  If a variable is not defined at one level of
 *	    the stack, it automatically inherits the value from the next lower
 *          item on the stack.
 *	
 *    - Experiment-wide "Trial Status" (class ScopedVariableContext): This object 
 *          contains status data for the entire experiment.  There is no 
 *          notion of a stack for this object; the data are "global" to the
 *          entire experiment.  Primarily "status" data which is set by the 
 *          running system should be stored here (e.g. "Trial Running" flags,
 *          "FixPoint On,"  "Current Eye Position" etc.)
 *
 *    - Eye Triggers  (class ExpandableList<EyeTrigger>): This object 
 *          defines how the monkey's eye position interacts with the 
 *          experiment's status variables.  Basically, the user registers 
 *          status variables which define hotspots on the screen, and registers
 *          other status varibles that are to be set when the monkey's eyes 
 *          enter the hotspot.  Because the "hotspot" locations and sizes are
 *	        controlled by status variables (which can in turn be set 
 *          dynamically by the user's state system), this system has the 
 *          flexibility to implement very complex, dynamic eye-based responses.
 *
 *    - MasterStimulusList  (class ExpandableList<Stimulus>): This object 
 *          contains an experiment-wide registry of stimulus frames. Any 
 *          stimulus which will be presented during an experiment should live
 *          here. Stimuli can be added here at startup, or can be set 
 *          dynamically at runtime (e.g. if stimuli are generated on the fly 
 *          based on feedback during the experiment). N.B.: if you want to 
 *          display a stimulus in more than one location simultaneously, 
 *          these need to be stored as separate entries in the list.
 *
 *    - StimulusGroups	(class ExpandableList<StimulusGroup>):  This object 
 *          contains StimulusGroup objects (themselves ExpandableLists) which
 *          provide a simple interface for breaking up stimuli into groups. 
 *          The grouping consists simply of a group index and a list of 
 *          MasterStimulusLIst indices.  Stimuli in MasterStimulusList can 
 *          belong to more than one group
 *
 *    - UI Interface (class UIInterface): An object which defines which 
 *          variables a user can get and set, what their ranges are, and 
 *          strings which describe the variable.  This object should be able 
 *          to be translated into a property-sheet style user interface at 
 *          runtime, giving users a runtime handle on certain info and status
 *          variables.
 *
 *    - Key Press Triggers (class ExandableList<KeyTrigger>): This object 
 *          defines how the user can interact directly with the experiment 
 *          via the keyboard. The user registers key presses with status 
 *          variables, "binding" the key to that status variable at runtime.
 *
 *    - StimulusDisplay (class StimulusDisplay): The main stimulus display 
 *          object.  The door is open to creating more than one of these,
 *          e.g. for fancy multi-display studies, or for desktop monitoring.
 *
 * History:
 * ??/??/?? ??? - Created
 * 04/14/04 PAJ - added accessor methods to get IntefaceSetting lists.
 * 04/15/04 PAJ - added method to return VariableRegistry
 * 05/17/04 PAJ - added method to return eye trigger list 
 * 07/06/2004 DDC - removed vestigial stimulus update code 
 */

///////////////////////////////////////////////
// GLOBAL DECLARATION (for now)
///////////////////////////////////////////////
namespace mw {
extern void setCurrentExperiment(Experiment *exp);
extern shared_ptr<Experiment> GlobalCurrentExperiment;

enum{M_INFO, M_STATUS};

class Experiment : public ContainerState {
    protected:

        // A pointer to the current active state within this experiment
    	weak_ptr<State> current_state;
	    
		// A pointer to the current active protocol
		shared_ptr<mw::Protocol> current_protocol;	
    	
		// The "global" variable context for this experiment
	    //mScopedVariableContext *experimentwide_variable_context;
		
		// A pointer to this experiment's variable registry
		shared_ptr<VariableRegistry> variable_registry; 
        
		// An abstract stimulus display associated with this experiment
		shared_ptr<StimulusDisplay> stimulus_display;
        
		int n_protocols; 
		
		// Info about this experiment
		std::string experimentName;
		std::string experimentPath;
		
		// Private methods to log changes in variable values
		void logChange(Variable *variable, const Data& data);
		void logChange(Variable *variable, const Data& data, MonkeyWorksTime timeUS);
        			
    public:
		Experiment(shared_ptr<VariableRegistry> variable_reg);
		virtual ~Experiment();
		
		
		// Generate variable contexts from the variable registry
	    void createVariableContexts();
		
		// Base State functionality that requires an Experiment to exist already
	   virtual void requestVariableContext(){ };
		
		//void addProtocol(shared_ptr<mw::Protocol> newprot);
	    //void addProtocol(int index, shared_ptr<mw::Protocol> newprot);

		
		// Accessors for current state system state
		// Are these used right now?
		virtual weak_ptr<State> getCurrentState();
	    virtual void setCurrentState(weak_ptr<State> newstate);


		// State inherited methods
		virtual void action();
		virtual weak_ptr<State> next();
		
		//virtual void announceIdentity();
		
		void update();	
		virtual void reset();

		
		
		// Accessors for Experiment info
		void setExperimentName(std::string);
        std::string getExperimentName();
        void setExperimentPath(std::string);
		std::string getExperimentPath();
		
		
		// Announce the local variables (typically on the behalf of another
		// paradigm component belonging to this experiment; e.g. when it goes
		// current).  The purpose of this method is to ensure that local
		// variable changes are logged in the event stream when as the 
		// control flows through the paradigm.
		virtual void announceLocalVariables(){
			ScopedVariableEnvironment::announceAll(); // ScopedVariableEnvironment method
		}
		
		
		// Accessors for stimulus display
	    shared_ptr<StimulusDisplay> getStimulusDisplay();
        void setStimulusDisplay(shared_ptr<StimulusDisplay> newdisplay);
        

		// Current mw::Protocol
	    void setCurrentProtocol(unsigned int protocol_number);
        void setCurrentProtocol(std::string);		
	    shared_ptr<mw::Protocol> getCurrentProtocol();	
	    shared_ptr<VariableRegistry> getVariableRegistry();


		// because SWIG eats ass
		virtual State *getStateInstance();
//		virtual void addState(shared_ptr<State> state);
		//virtual void addState(int index, shared_ptr<State> state);
//		
		
		virtual void finalize(std::map<std::string, std::string> parameters,
												mwComponentRegistry *reg) {
			current_state = getSelfPtr<State>();
			experiment = getSelfPtr<Experiment>();
			environment = getSelfPtr<ScopedVariableEnvironment>();
			createVariableContexts();
		}
		
		virtual void addChild(std::map<std::string, std::string> parameters,
								mwComponentRegistry *reg,
								shared_ptr<mw::Component> child){
			
			ContainerState::addChild(parameters, reg, child);
			
			if(current_protocol == NULL) {
				shared_ptr<State> first_protocol = list->operator[](0);
				current_protocol = boost::dynamic_pointer_cast<mw::Protocol, State>(first_protocol); // TODO: remove up-cast
				current_state = weak_ptr<State>(first_protocol);
			}
			n_protocols++;
		}
};


		
class ExperimentFactory : public ComponentFactory {

	public:
		
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												mwComponentRegistry *reg) {
		GlobalCurrentExperiment = shared_ptr<Experiment>(new Experiment(GlobalVariableRegistry));
	
		// TODO
		// Not sure if this is okay order-wise...
		// This may need to be a separate parser directive
		//GlobalCurrentExperiment->createVariableContexts();
		prepareStimulusDisplay();
		
		shared_ptr<mw::Component> experiment = dynamic_pointer_cast<mw::Component, Experiment>(GlobalCurrentExperiment);		
		
		if(!parameters["tag"].empty()){
			GlobalCurrentExperiment->setExperimentName(parameters["tag"]);
		}
		return experiment;
	}
		
};

}


#endif
