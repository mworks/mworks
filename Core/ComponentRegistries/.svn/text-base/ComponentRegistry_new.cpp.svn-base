/*
 *  mwComponentRegistry.cpp
 *  Experimental Control with Cocoa UI
 *
 *  Created by David Cox on Fri Dec 27 2002.
 *  Copyright (c) 2002 __MyCompanyName__. All rights reserved.
 *
 */

#include "Utilities.h"
#include "ComponentRegistry_new.h"
#include "ComponentFactoryException.h"
#include "GlobalVariable.h"
#include "UnresolvedReferenceVariable.h"
#include "BlockAndProtocol.h"
#include "States.h"
#include "TrialBuildingBlocks.h"
#include "ScheduledActions.h"
#include "PulseAction.h"
#include "CancelScheduledActionsAction.h"
#include "StandardSounds.h"
#include "Averagers.h"
#include "SimpleStaircase.h"
#include "DummyIODevice.h"
#include "StandardSounds.h"
#include "XMLParser.h"
#include "BiasMonitor.h"


#include <boost/regex.hpp>
using namespace mw;

shared_ptr<mwComponentRegistry> mwComponentRegistry::shared_component_registry;

mwComponentRegistry::mwComponentRegistry() {
	// register all of the built-ins here?
	
	registerFactory("experiment", new ExperimentFactory());
	registerFactory("protocol", new ProtocolFactory());
	registerFactory("block", new BlockFactory());
	registerFactory("list", new GenericListStateFactory());
	registerFactory("trial", new TrialFactory());
	
	
	registerFactory("task_system", new TaskSystemFactory());
	registerFactory("task_system_state", new TaskSystemStateFactory());
	
	registerFactory("variable", new VariableFactory());
	registerFactory("variable/selection", new SelectionVariableFactory());
	
	
	registerFactory("action/schedule", new ScheduledActionsFactory());
	registerFactory("action/if", new IfFactory());
	registerFactory("action/cancel_scheduled_action", new CancelScheduledActionFactory());
	
	
	registerFactory("action/assignment", new AssignmentFactory());
	registerFactory("action/pulse", new PulseFactory());
	registerFactory("action/report", new ReportStringFactory());
	registerFactory("action/assert", new AssertionActionFactory());
	registerFactory("action/next_selection", new NextVariableSelectionFactory());
	registerFactory("action/set_timebase", new SetTimeBaseFactory());
	registerFactory("action/start_timer", new StartTimerFactory());
	registerFactory("action/wait", new WaitFactory());
	registerFactory("action/queue_stimulus", new QueueStimulusFactory());
	registerFactory("action/show_stimulus", new QueueStimulusFactory());
	registerFactory("action/live_queue_stimulus", new LiveQueueStimulusFactory());
	registerFactory("action/dequeue_stimulus", new DequeueStimulusFactory());
	registerFactory("action/hide_stimulus", new DequeueStimulusFactory());
	registerFactory("action/bring_stimulus_to_front", new BringStimulusToFrontFactory());
	registerFactory("action/send_stimulus_to_back", new SendStimulusToBackFactory());
	registerFactory("action/update_stimulus_display", new UpdateStimulusDisplayFactory());
	registerFactory("action/play_sound", new PlaySoundFactory());
	registerFactory("action/stop_sound", new StopSoundFactory());
	registerFactory("action/pause_sound", new PauseSoundFactory());
	registerFactory("action/start_device_io", new StartDeviceIOFactory());
	registerFactory("action/stop_device_io", new StopDeviceIOFactory());
//	registerFactory("action/fake_monkey_saccade_to_location", new FakeMonkeySaccadeToLocationFactory());	
	registerFactory("action/reset_selection", new ResetSelectionFactory());
	registerFactory("action/accept_selections", new AcceptSelectionsFactory());
	registerFactory("action/reject_selections", new RejectSelectionsFactory());
	registerFactory("action/begin_calibration_average", new StartAverageCalibrationSampleFactory());
	registerFactory("action/end_calibration_average_and_ignore", new EndAverageAndIgnoreFactory());
	registerFactory("action/end_calibration_average_and_take_sample", new EndAverageAndTakeCalibrationSampleFactory());
	registerFactory("action/update_calibration", new CalibrateNowFactory());
	registerFactory("action/clear_calibration", new ClearCalibrationFactory());

	// transitions
	registerFactory("transition", new TransitionFactory());
	
	// var transform adapters
	registerFactory("averager", new AveragerUserFactory());
	registerFactory("calibrator/standard_eye_calibrator", new EyeCalibratorFactory());
	registerFactory("calibrator/linear_eye_calibrator", new LinearEyeCalibratorFactory());
	registerFactory("filter/basic_eye_monitor", new EyeStatusMonitorVer1Factory());
	registerFactory("advanced_eye_monitor", new EyeStatusMonitorVer2Factory());
	registerFactory("filter/boxcar_filter_1d", new Filter_BoxcarFilter1DFactory());
	registerFactory("linear_filter_1d", new Filter_LinearFilter1DFactory());
	registerFactory("staircase", new SimpleStaircaseOptimizerFactory());
    registerFactory("bias_monitor", new BiasMonitorFactory());
	
	// IO devices
	registerFactory("iodevice/dummy", new DummyIODeviceFactory());
	//registerFactory("iodevice/niusb6009", new NIUSB6009Factory());
	
	registerFactory("iochannel", new IOChannelRequestFactory());
	
	// stimuli
	registerFactory("stimulus/blank_screen", new BlankScreenFactory());
	registerFactory("stimulus/image_file", new ImageStimulusFactory());
	registerFactory("stimulus/fixation_point", new FixationPointFactory());
	registerFactory("stimulus_group", new StimulusGroup());
	
	// sounds
	registerFactory("sound/wav_file", new WavFileSoundFactory());
    
}

// Factory-oriented methods
void mwComponentRegistry::registerFactory(std::string type_name, 
										 ComponentFactory *_factory){
	shared_ptr<ComponentFactory> factory(_factory);
	factories[type_name] = factory;
}

shared_ptr<ComponentFactory> mwComponentRegistry::getFactory(std::string type_name){
	
	shared_ptr<ComponentFactory> factory = factories[type_name];
	
	
	if(factory == NULL){
		// try splitting
		std::vector<std::string> split_vector;
		split(split_vector, type_name, is_any_of("/"));
		
		factory = factories[split_vector[0]];
	}
	
	
	return factory;
}


// Instance-oriented methods

// Use a registered factory to create a new object and register the
// result in the instances map
// Returns true on success, false on failure
shared_ptr<mw::Component> mwComponentRegistry::registerNewObject(std::string tag_name, 
															 std::string type_name, 
															 std::map<std::string, std::string> parameters) {
	
	shared_ptr<mw::Component> obj = this->createNewObject(type_name, parameters);
	
	if(obj != NULL){
		// put it into the instances map
		registerObject(tag_name, obj);
	}

	return obj;
}

shared_ptr<mw::Component> mwComponentRegistry::createNewObject(const std::string &type_name, 
														   const std::map<std::string, std::string> &parameters){
	// get the factory
	shared_ptr<ComponentFactory> factory = getFactory(type_name);
	
	// make sure it is valid
	if(factory == NULL){
		// do something
		throw SimpleException("No factory for object type", type_name);
	}
	
	// create the new object
	shared_ptr<mw::Component> obj;
	try {
		obj = factory->createObject(parameters, this);
	} catch (std::exception& e){
		merror(M_PARSER_MESSAGE_DOMAIN, e.what());
	}
	
	return obj;
}


void mwComponentRegistry::registerObject(std::string tag_name, shared_ptr<mw::Component> component, bool force){	
	
	if(tag_name.empty()){
		throw SimpleException("Attempt to register a component with an empty name");
	}
	if(component == NULL){
		throw SimpleException("Attempt to register empty component", tag_name);
	}
	
	
//	if(getObject<mw::Component>(tag_name) != NULL) {
//		std::string error_string = "Registering object of tag name (" + tag_name + ") when an object with that tag is already registered.";
//		merror(M_PARSER_MESSAGE_DOMAIN, error_string.c_str());
//		error_string = "This is not inherently bad, but be cautious.  Actions can have the same name as other actions, but they should be identical.";
//		merror(M_PARSER_MESSAGE_DOMAIN, error_string.c_str());
//		error_string = "Disparate objects (i.e. a state and a variable) shouldn't have the same name.";
//		merror(M_PARSER_MESSAGE_DOMAIN, error_string.c_str());
//	}
    // check to see if a component has already been registered with this tag name
/*    if(!force && getObject<mw::Component>(tag_name) != NULL){
        AmbiguousComponentReference *ref = new AmbiguousComponentReference();
        
        // Register the effending components here (we'll want access to them later, should the
        // ambiguity lead to problems
        ref->addAmbiguousComponent(component);
        ref->addAmbiguousComponent(instances[tag_name]);
        
        shared_ptr<mw::Component> ambiguous_component(ref);
        
        // Replace the previous entry listed at this tagname with the 
        // ambiguous component reference.  Both the original and the new object
        // are now effectively "anonymous" (parents can access them, but it is
        // not possible to access them by name).  In most cases this will not be
        // a problem, but in some cases it will.
        instances[tag_name] = ambiguous_component;
        // could also throw here
        return;
    }*/
    
	instances[tag_name] = component;
	
	tagnames_by_id[component->getCompactID()] = tag_name;
	
	component->setTag(tag_name);
	weak_ptr<mw::Component> self_reference(component);
	component->setSelfPtr(self_reference);
}

void mwComponentRegistry::registerAltObject(const std::string &tag_name, shared_ptr<mw::Component> component){	
	
	if(tag_name.empty()){
		throw SimpleException("Attempt to register an 'alt' component with an empty name");
	}
	if(component == NULL){
		throw SimpleException("Attempt to register empty 'alt' component", tag_name);
	}
	
	instances[tag_name] = component;	
}

void mwComponentRegistry::registerStimulusNode(const std::string &tag_name, shared_ptr<StimulusNode> stimNode) {
	stimulus_nodes[tag_name] = stimNode;
}

// Instance lookups with some extra parsing smarts
shared_ptr<Variable>	mwComponentRegistry::getVariable(std::string expression){


	// Check to see if it can be resolved, or if it will need to be resolved
	// at runtime
	smatch unresolved_match;
	boost::regex u1("^(.*?\\$.+?)$");
	bool unresolved = boost::regex_match(expression, unresolved_match, u1);
	if(unresolved){
		shared_ptr<mwComponentRegistry> registry = mwComponentRegistry::getSharedRegistry();
		shared_ptr<Variable> unresolved_var(new UnresolvedReferenceVariable(expression, registry));
		return unresolved_var;
	}

	smatch matches1, matches2, matches3;
	boost::regex r1(".*?[\\*\\!\\+\\-\\=\\/\\&\\|\\%\\>\\<\\(\\)].*?");
	//boost::regex r1("(\\*|\\!|\\+|\\-|\\=|\\/|\\&|\\||\\%|\\>|\\<|\\(|\\))");
	boost::regex r2(".*?((\\#AND)|(\\#OR)|(\\#GT)|(\\#LT)|(\\#GE)|(\\#LE)|(\\Wms)|([^a-zA-z#]s)|(\\Wus)).*");
	boost::regex r3("^\\s*\\d*\\.?\\d*\\s*(ms|us|s)?\\s*$");
	boost::regex r4("^\\s*\\.?\\d*\\s*(ms|us|s)?\\s*$");
	bool b1, b2, b3;
	b1 = boost::regex_match(expression, matches1, r1); 
	b2 = boost::regex_match(expression, matches2, r2);
	b3 = boost::regex_match(expression, matches3, r3);
	if(b1 || b2 || b3){
		shared_ptr<Variable> expression_var(new ParsedExpressionVariable(expression));	
		return expression_var;
	} 
	
	smatch strip_match;
	boost::regex strip_it("^\\s*(.+?)\\s*$");
	boost::regex_match(expression, strip_match, strip_it); 
	
	shared_ptr<Variable> var = GlobalVariableRegistry->getVariable(strip_match[1]);
	return var;
}

// An alternate getVariable call that includes a default value
shared_ptr<Variable> mwComponentRegistry::getVariable(std::string expression,
													  std::string default_value){

	if(expression.empty()){
		return getVariable(default_value);
	} else {
		return getVariable(expression);
	}
}

shared_ptr<mw::StimulusNode>	mwComponentRegistry::getStimulus(std::string expression){
	
	using namespace boost;
	
	// regex for parsing the stimulus string
	regex stimulus_regex("(.+?)(\\[(.+)\\])?"); 
	
	smatch matches;
	try{
		
		regex_match(expression, matches, stimulus_regex); 
		// matches[0] contains the whole string 
		// matches[1] contains the stimulus "stem" 
		// matches[2] contains the index expression (if there is one), 
		//			  including brackets. 
		// matches[3] contains the index expression (if there is one), 
		//			  without brackets.
		
   } catch (regex_error& e) {
		throw SimpleException("Regex error during stimulus parsing (regex_error exception)", e.what());
   }
   
   // Something is wrong if there aren't at least two strings in matches
   if(matches.size() == 1){
		// TODO: throw
		throw SimpleException("Regex error during stimulus parsing (not enough matches)");
	}
   
	// This the part before any brackets
	std::string stem = matches[1];
	
	std::string remainder = matches[2];
	if(remainder.empty()){
		return getStimulusNode(stem);
	}
	
	// If we're still going, from this point forward, the stem refers to a 
	// stimulus group
	shared_ptr<StimulusGroup> stimulus_group = getObject<StimulusGroup>(stem);
	
	shared_ptr<StimulusNodeGroup> stimulus_node_group = getObject<StimulusNodeGroup>(stem + ":node");
	
	if(stimulus_node_group == NULL){
		stimulus_node_group = shared_ptr<StimulusNodeGroup>(new StimulusNodeGroup(stimulus_group));
		
		shared_ptr<mw::Component> newStimulusNodeGroup = dynamic_pointer_cast<mw::Component, StimulusNodeGroup>(stimulus_node_group);
		registerObject(stem + ":node", newStimulusNodeGroup);
	}
	
	if(stimulus_group == NULL){
		// TODO: throw?
		throw SimpleException("Failed to find stimulus group", stem);
	}
	
	// This is the stuff inside the outer brackets
	std::string index_pattern = matches[3];
	

	// This is where we are going to put the string when we are done with it
	std::string index_expression;
	
	// A regex to determine if the stimulus is multidimensional, e.g. stim[1][2]
	regex multi_dimensional_group_regex(".+\\]\\s*\\[.+");
	if( regex_match(index_pattern, multi_dimensional_group_regex) ){
		
		// If it's multidimensional, we need to split up the contents of the
		// outer brackets and multiply/add them into a c-style multidimensional
		// array index. 

		// We're going to use this string stream for syntactic convenience in
		// concatenating things together.
		std::ostringstream index_expression_stream;
		index_expression_stream << "0";  // get things started
		
		
		regex index_regex("\\]\\s*\\[");

		// split the index strings
		sregex_token_iterator i_t(index_pattern.begin(), 
									   index_pattern.end(),
									   index_regex, -1);
		sregex_token_iterator j_t;
		
		vector<string> indices;
		while(i_t != j_t){
			indices.push_back(*i_t++);
		}

		if(indices.size() != stimulus_group->getNDimensions()) {
			throw SimpleException("Illegal stimulus dimension reference", stem);			
		}
		
		for(unsigned int i = 0; i < indices.size(); i++){
			index_expression_stream << "+((" << boost::algorithm::trim_copy(indices.at(i)) << ")";
			
			for(unsigned int j = i + 1; j < indices.size(); j++){			
				index_expression_stream << "*("; 
				index_expression_stream << stimulus_group->dimensionSize(j);
				index_expression_stream << ")";
			}
			index_expression_stream << ")";
		}
		
		index_expression = index_expression_stream.str();
	} else {
		// if not multi-dimensional, the work is easier
		index_expression = index_pattern;
	}
	
	// Parse the expression
	shared_ptr<Variable> index = getVariable(index_expression);
	
	// Create a "node group" to go with the stimulus group
	//shared_ptr<StimulusNodeGroup> 
	//	stimulus_node_group(new StimulusNodeGroup(stimulus_group));
	
	// Create the relevant reference
	shared_ptr<StimulusGroupReferenceNode> 
				refnode(new StimulusGroupReferenceNode(stimulus_node_group, 
														index));
	
	return refnode;
}

bool mwComponentRegistry::getBoolean(std::string expression){

	// cheap and dirty for now
	if(expression == "YES") return true;
	if(expression == "yes") return true;
	
	if(expression == "NO") return false;
	if(expression == "no") return false;
	
	return boost::lexical_cast<bool>(expression);
}


Data mwComponentRegistry::getNumber(std::string expression, GenericDataType type){

	
	ParsedExpressionVariable e(expression);
	
	Data value = e.getValue();
	
	switch (type){
	
		case M_FLOAT:
			return Data(value.getFloat());
		case M_INTEGER:
			return Data(value.getInteger());
		case M_STRING:
			return Data(value.getString());
		case M_BOOLEAN:
			return Data(value.getBool());
		default:
			return Data(value);			
	}
	
	return Data(value);
		
/*	
	
	std::string final_expression("");
	int modifier = 1;

	// now, do a quick and find on "us", "ms" and "s" 
	boost::regex r("(.*?\\W)(us|ms|s)");
	smatch matches;
	std::string modifier_part("");
	
	
	if(regex_match(expression, matches, r)){
		final_expression = matches[1];
		modifier_part = matches[2];
		
		if(modifier_part == "ms"){
			modifier = 1000;
		} else if(modifier_part == "s"){
			modifier = 1000000;
		}
	} else {
		final_expression = expression;
	}

	double result = boost::lexical_cast<double>(final_expression);
	result *= modifier;
	
	return Data(result);*/
}


/*    my ($string) = @_;
    my $value = 0;
    
    if($string =~ m"(\+|\-|\*|\/|%)"){   # it's an expression
	
	# TODO: should validate stuff better, maybe use Math::Expression...
	
	$string =~ s{(\d+)\s*us}{ ($1 * 1) };
	$string =~ s{(\d+)\s*ms}{ ($1 * 1000) };
	$string =~ s{(\d+)\s*s}{ ($1 * 1000000) };
	$string =~ s{pi}{3.14159265};
	
	$string = "\$value = $string + 0;";
	
	eval($string);
	
	if(!defined($value)){
	    #pprint("Invalid arithmetic expression for variable definition");
	    $value = 0;
	}
	
    } elsif($string =~ m"^([\.\d]+)\s*(s|ms|us)"){   # it's a time
	$value = $1 + 0;
	if($2 eq "s"){
	    $value *= 1000000;
	} elsif($2 eq "ms"){
	    $value *= 1000;
	}
	
	
    } elsif($string =~ m"^([\.\d]+)"){   # it's a number
	$value = $string + 0;
    } elsif($string =~ m"true"i || $string =~ m"yes"i){
	$value = 1;
    } elsif($string =~ m"false"i || $string =~ m"no"i){
	$value = 0;
    } elsif($string eq '') {
	parser_warning(0, "Empty string for variable");
	undef($value);
    } else {
	parser_warning(0, "Don't know how to convert \"$string\" to a variable.");
	undef($value);
    }	
    
    return $value;


}*/

boost::filesystem::path mwComponentRegistry::getPath(std::string working_path,
													std::string expression){

	std::string squashed = XMLParser::squashFileName(expression);
	return expandPath(working_path, squashed);
}

