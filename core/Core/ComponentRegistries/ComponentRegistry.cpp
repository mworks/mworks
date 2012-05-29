/*
 *  ComponentRegistry.cpp
 *  Experimental Control with Cocoa UI
 *
 *  Created by David Cox on Fri Dec 27 2002.
 *  Copyright (c) 2002 __MyCompanyName__. All rights reserved.
 *
 */

#include "Utilities.h"
#include "ComponentRegistry.h"
#include "ComponentFactoryException.h"
#include "GlobalVariable.h"
#include "UnresolvedReferenceVariable.h"
#include "BlockAndProtocol.h"
#include "States.h"
#include "TrialBuildingBlocks.h"
#include "DynamicStimulusActions.h"
#include "ScheduledActions.h"
#include "PulseAction.h"
#include "StandardSounds.h"
#include "Averagers.h"
#include "SimpleStaircase.h"
#include "DummyIODevice.h"
#include "LegacyIODevice.h"
#include "StandardSounds.h"
#include "XMLParser.h"
#include "BiasMonitor.h"
#include "StandardComponentFactory.h"
#include "StandardStimulusFactory.h"


BEGIN_NAMESPACE_MW


shared_ptr<ComponentRegistry> ComponentRegistry::shared_component_registry;

ComponentRegistry::ComponentRegistry() :
        r1(".*?[\\*\\!\\+\\-\\=\\/\\&\\|\\%\\>\\<\\(\\)\"].*?"),
        r2(".*?((\\#AND)|(\\#OR)|(\\#GT)|(\\#LT)|(\\#GE)|(\\#LE)|(\\Wms)|([^a-zA-z#]s)|(\\Wus)).*"),
        r3("^\\s*\\d*\\.?\\d*\\s*(ms|us|s)?\\s*$"),
        r4("^\\s*\\.?\\d*\\s*(ms|us|s)?\\s*$"),
        u1("^(.*?\\$.+?)$"),
        strip_it("^\\s*(.+?)\\s*$")
{

	// register all of the built-ins here
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
	
	
	registerFactory("action/assignment", new AssignmentFactory());
	registerFactory("action/pulse", new PulseFactory());
    registerFactory<StandardComponentFactory, ReportString>();
    registerFactory<StandardComponentFactory, AssertionAction>();
	registerFactory("action/next_selection", new NextVariableSelectionFactory());
	registerFactory("action/set_timebase", new SetTimeBaseFactory());
	registerFactory("action/start_timer", new StartTimerFactory());
	registerFactory("action/wait", new WaitFactory());
	registerFactory("action/load_stimulus", new LoadStimulusFactory());
    registerFactory("action/unload_stimulus", new UnloadStimulusFactory());
    registerFactory("action/queue_stimulus", new QueueStimulusFactory());
	registerFactoryAlias("action/queue_stimulus", "action/show_stimulus");
	registerFactory("action/live_queue_stimulus", new LiveQueueStimulusFactory());
	registerFactory("action/dequeue_stimulus", new DequeueStimulusFactory());
	registerFactoryAlias("action/dequeue_stimulus", "action/hide_stimulus");
	registerFactory("action/bring_stimulus_to_front", new BringStimulusToFrontFactory());
	registerFactory("action/send_stimulus_to_back", new SendStimulusToBackFactory());
	registerFactory("action/update_stimulus_display", new UpdateStimulusDisplayFactory());
	registerFactory("action/play_sound", new PlaySoundFactory());
	registerFactory("action/stop_sound", new StopSoundFactory());
	registerFactory("action/pause_sound", new PauseSoundFactory());
	registerFactory("action/start_device_io", new StartDeviceIOFactory());
	registerFactory("action/stop_device_io", new StopDeviceIOFactory());
	registerFactory("action/reset_selection", new ResetSelectionFactory());
	registerFactory("action/accept_selections", new AcceptSelectionsFactory());
	registerFactory("action/reject_selections", new RejectSelectionsFactory());
    registerFactory<StandardComponentFactory, StopExperiment>();
	registerFactory("action/take_calibration_sample", new TakeCalibrationSampleNowFactory());
	registerFactory("action/begin_calibration_average", new StartAverageCalibrationSampleFactory());
	registerFactory("action/end_calibration_average_and_ignore", new EndAverageAndIgnoreFactory());
	registerFactory("action/end_calibration_average_and_take_sample", new EndAverageAndTakeCalibrationSampleFactory());
	registerFactory("action/update_calibration", new CalibrateNowFactory());
	registerFactory("action/clear_calibration", new ClearCalibrationFactory());

    registerFactory<StandardComponentFactory, PlayDynamicStimulus>();
    registerFactory<StandardComponentFactory, StopDynamicStimulus>();
    registerFactory<StandardComponentFactory, PauseDynamicStimulus>();
    registerFactory<StandardComponentFactory, UnpauseDynamicStimulus>();
            
            
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
    registerFactory<StandardComponentFactory, DummyIODevice>();
    registerFactory<StandardComponentFactory, IOChannelRequest>();
	
	// stimuli
    registerFactory<StandardStimulusFactory, BlankScreen>();
    registerFactory<StandardStimulusFactory, ImageStimulus>();
    registerFactory<StandardStimulusFactory, FixationPoint>();
    registerFactory<StandardComponentFactory, StimulusGroup>();
	
	// sounds
    registerFactory<StandardComponentFactory, WavFileSound>();
  
  
  // cache these as members, since they will otherwise be created and destroyed a bazillion times
  //boost::regex r1(".*?[\\*\\!\\+\\-\\=\\/\\&\\|\\%\\>\\<\\(\\)].*?");
//	//boost::regex r1("(\\*|\\!|\\+|\\-|\\=|\\/|\\&|\\||\\%|\\>|\\<|\\(|\\))");
//	boost::regex r2(".*?((\\#AND)|(\\#OR)|(\\#GT)|(\\#LT)|(\\#GE)|(\\#LE)|(\\Wms)|([^a-zA-z#]s)|(\\Wus)).*");
//	boost::regex r3("^\\s*\\d*\\.?\\d*\\s*(ms|us|s)?\\s*$");
//	boost::regex r4("^\\s*\\.?\\d*\\s*(ms|us|s)?\\s*$");
    
}


// Factory-oriented methods


void ComponentRegistry::registerFactory(const std::string &type_name, shared_ptr<ComponentFactory> factory) {
    shared_ptr<ComponentFactory> existing_factory = factories[type_name];
    if(existing_factory != NULL){
        throw ComponentFactoryConflictException(type_name);
	}
    
    factories[type_name] = factory;
}


shared_ptr<ComponentFactory> ComponentRegistry::getFactory(const std::string &type_name) {
	shared_ptr<ComponentFactory> factory(factories[type_name]);
	
	if (!factory) {
		// try splitting
		std::vector<std::string> split_vector;
		split(split_vector, type_name, is_any_of("/"));
		factory = factories[split_vector[0]];
	}
	
	if (!factory) {
		throw SimpleException("No factory for object type", type_name);
	}
	
	return factory;
}


// Instance-oriented methods

// Use a registered factory to create a new object and register the
// result in the instances map
// Returns true on success, false on failure
shared_ptr<mw::Component> ComponentRegistry::registerNewObject(std::string tag_name, 
															 std::string type_name, 
															 std::map<std::string, std::string> parameters) {
	
	shared_ptr<mw::Component> obj = this->createNewObject(type_name, parameters);
	
	if(obj != NULL){
		// put it into the instances map
		registerObject(tag_name, obj);
	} else {
        mwarning(M_PARSER_MESSAGE_DOMAIN, "Null object created");
    }
	return obj;
}

shared_ptr<mw::Component> ComponentRegistry::createNewObject(const std::string &type_name, 
														   const std::map<std::string, std::string> &parameters){
	// get the factory
	shared_ptr<ComponentFactory> factory = getFactory(type_name);
	
	// create the new object
    return factory->createObject(parameters, this);
}


void ComponentRegistry::registerObject(std::string tag_name, shared_ptr<mw::Component> component, bool force){	
	
	if(tag_name.empty()){
		throw SimpleException("Attempt to register a component with an empty name");
	}
	if(component == NULL){
		throw SimpleException("Attempt to register empty component", tag_name);
	}    
        
    // If the tag name is already registered
    if(instances.find(tag_name) != instances.end() && instances[tag_name] != NULL){
        
        shared_ptr<Component> preexisting = instances[tag_name];
        
        if(preexisting == NULL){
            throw SimpleException("Attempt to redefine an existing component.  All names within an experiment must be unique", tag_name);
        }

        shared_ptr<AmbiguousComponentReference> ambiguous_component;
        if(preexisting->isAmbiguous()){
            ambiguous_component = dynamic_pointer_cast<AmbiguousComponentReference>(preexisting);
        } else {
            ambiguous_component = shared_ptr<AmbiguousComponentReference>(new AmbiguousComponentReference());
        }
        
        ambiguous_component->addAmbiguousComponent(component);
        
        instances[tag_name] = dynamic_pointer_cast<AmbiguousComponentReference, Component>(ambiguous_component);
        
        return;
    }
    
	instances[tag_name] = component;
	
	tagnames_by_id[component->getCompactID()] = tag_name;
	
	component->setTag(tag_name);
	//weak_ptr<mw::Component> self_reference(component);
	//component->setSelfPtr(self_reference);
}

void ComponentRegistry::registerAltObject(const std::string &tag_name, shared_ptr<mw::Component> component){	
	
	if(tag_name.empty()){
		throw SimpleException("Attempt to register an 'alt' component with an empty name");
	}
	if(component == NULL){
		throw SimpleException("Attempt to register empty 'alt' component", tag_name);
	}
	
	instances[tag_name] = component;	
}

void ComponentRegistry::registerStimulusNode(const std::string &tag_name, shared_ptr<StimulusNode> stimNode) {
	stimulus_nodes[tag_name] = stimNode;
}

// Instance lookups with some extra parsing smarts
shared_ptr<Variable>	ComponentRegistry::getVariable(std::string expression){

    shared_ptr<Variable> test = variable_cache[expression]; 
    if(test != NULL){
        return test;
    }
  
	// Check to see if it can be resolved, or if it will need to be resolved
	// at runtime
	smatch unresolved_match;

	bool unresolved = boost::regex_match(expression, unresolved_match, u1);
	if(unresolved){
		shared_ptr<ComponentRegistry> registry = ComponentRegistry::getSharedRegistry();
		shared_ptr<Variable> unresolved_var(new UnresolvedReferenceVariable(expression, registry));
		return unresolved_var;
	}

	smatch matches1, matches2, matches3;
	
	bool b1, b2, b3;
	b1 = boost::regex_match(expression, matches1, r1); 
	b2 = boost::regex_match(expression, matches2, r2);
	b3 = boost::regex_match(expression, matches3, r3);
	if(b1 || b2 || b3){
        // TODO try
		shared_ptr<Variable> expression_var(new ParsedExpressionVariable(expression));	
		return expression_var;
	} 
	
	smatch strip_match;
	boost::regex_match(expression, strip_match, strip_it); 
	
	shared_ptr<Variable> var = global_variable_registry->getVariable(strip_match[1]);
  
    if(var == NULL){
        throw UnknownVariableException(strip_match[1]);
    }
  
    // cache/hash the variable for fast access
    variable_cache[expression] = var;
  
	return var;
}

// An alternate getVariable call that includes a default value
shared_ptr<Variable> ComponentRegistry::getVariable(std::string expression,
													  std::string default_value){

	if(expression.empty()){
		return getVariable(default_value);
	} else {
		return getVariable(expression);
	}
}

shared_ptr<mw::StimulusNode>	ComponentRegistry::getStimulus(std::string expression){
	
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
		throw FatalParserException("Regex error during stimulus parsing (regex_error exception)", e.what());
   }
   
   // Something is wrong if there aren't at least two strings in matches
   if(matches.size() == 1){
		// TODO: throw
		throw FatalParserException("Regex error during stimulus parsing (not enough matches)");
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
	
    if(stimulus_group == NULL){
        throw FatalParserException("Unknown stimulus group", stem);
    }
    
	shared_ptr<StimulusNodeGroup> stimulus_node_group = getObject<StimulusNodeGroup>(stem + ":node");
	
	if(stimulus_node_group == NULL){
		stimulus_node_group = shared_ptr<StimulusNodeGroup>(new StimulusNodeGroup(stimulus_group));
		
		shared_ptr<mw::Component> newStimulusNodeGroup = dynamic_pointer_cast<mw::Component, StimulusNodeGroup>(stimulus_node_group);
		registerObject(stem + ":node", newStimulusNodeGroup);
	}
	
	if(stimulus_group == NULL){
		throw FatalParserException("Unknown stimulus group", stem);
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
			throw FatalParserException("Illegal stimulus dimension reference", stem);			
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
	
    
    if(index_expression.empty()){
        FatalParserException f("Empty expression indexing stimulus group");
        f << stimulus_group_error_info(stimulus_group);
        throw f;
    }
    
    shared_ptr<Variable> index_var;
    
	// Parse the index expression
    try {
        index_var = getVariable(index_expression);
	
    } catch(UnknownExpressionException& e){
        
        FatalParserException f("Invalid index to stimulus group");
        f << expression_error_info(index_expression.c_str());
        
        throw f;
    }
    
	// Create a "node group" to go with the stimulus group
	//shared_ptr<StimulusNodeGroup> 
	//	stimulus_node_group(new StimulusNodeGroup(stimulus_group));
	
	// Create the relevant reference
	shared_ptr<StimulusGroupReferenceNode> 
				refnode(new StimulusGroupReferenceNode(stimulus_node_group, 
														index_var));
	
	return refnode;
}

bool ComponentRegistry::getBoolean(std::string expression){

	// cheap and dirty for now
	if(expression == "YES") return true;
	if(expression == "yes") return true;
	
	if(expression == "NO") return false;
	if(expression == "no") return false;
	
	return boost::lexical_cast<bool>(expression);
}


Datum ComponentRegistry::getNumber(std::string expression, GenericDataType type){

  std::pair<std::string, GenericDataType> cacheKey(expression, type);
  shared_ptr<Datum> test = data_cache[cacheKey];
  if(test != NULL){
    return *test;
  }
  
  double doubleValue;
  long longValue;
  bool boolValue;
  Datum value;
  try {
    switch(type){
      case M_FLOAT:
      case M_INTEGER:
      case M_BOOLEAN:
            doubleValue = lexical_cast<double>(expression);
            if (M_FLOAT == type) {
                value = Datum(doubleValue);
            } else if (M_INTEGER == type) {
                longValue = (long)doubleValue;
                if ((double)longValue != doubleValue) {
                    
                    throw NonFatalParserException("invalid integer literal", expression.c_str());
                }
                value = Datum(longValue);
            } else {
                boolValue = (bool)doubleValue;
                if ((double)boolValue != doubleValue) {
                    
                    throw NonFatalParserException("invalid boolean literal", expression.c_str());
                }
                value = Datum(boolValue);
            }
            break;
      case M_STRING:
          value = Datum(string(expression));
          break;
      default:
          throw NonFatalParserException("Attempt to cast a number of invalid type");
    }
    
    data_cache[cacheKey] = shared_ptr<Datum>(new Datum(value));
    return value;
  } catch (NonFatalParserException& e){
      // Until we work out how to effectively flag these issues, treat them as fatal errors
      throw FatalParserException(e.what());
  } catch (boost::bad_lexical_cast& e){
    // no biggie, we can do this the hard(er) way
  }
  
  
	ParsedExpressionVariable e(expression);
	
	value = e.getValue();
	
	switch (type){
	
		case M_FLOAT:
			return Datum(value.getFloat());
		case M_INTEGER:
			return Datum(value.getInteger());
		case M_STRING:
			return Datum(value.getString());
		case M_BOOLEAN:
			return Datum(value.getBool());
		default:
			return Datum(value);			
	}
	
	return Datum(value);
		
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
	
	return Datum(result);*/
}


string ComponentRegistry::getValueForAttribute(string attribute,
                                               map<string, string>& parameters){
    
    map<string,string>::iterator attr_iter = parameters.find(attribute);
    if(attr_iter == parameters.end()){
        
        string reference_id("<unknown object>");
        if(parameters.find("reference_id") != parameters.end()){
            reference_id = parameters["reference_id"];
        }
        
        string tag = parameters["tag"];
        if(tag.empty()){
            tag = "unknown name";
        }
        
        throw MissingAttributeException(reference_id, (boost::format("Object <%s> missing required attribute '%s'")% tag % attribute).str());
    }
    
    return (*attr_iter).second;
}



shared_ptr<Variable> ComponentRegistry::getVariableForAttribute(string attribute,
                                                               map<string, string>& parameters,
                                                               string default_expression){
    
    
    // if there's a default value specified, catch missing attribute exceptions
    // and use the default expression instead
    if(!default_expression.empty() && default_expression.size() > 0){
        try {
            string val = getValueForAttribute(attribute, parameters);
            return getVariable(val, default_expression);
        } catch (MissingAttributeException){
            return getVariable(default_expression);
        }
    }
    
    string val = getValueForAttribute(attribute, parameters);
    return getVariable(val, default_expression);
    
}

Datum ComponentRegistry::getNumberForAttribute(string attribute,
                                              map<string, string>& parameters,
                                              GenericDataType datatype){
    
    string val = getValueForAttribute(attribute, parameters);
    
    return getNumber(val, datatype);
}

Datum ComponentRegistry::getNumberForAttribute(string attribute,
                                              map<string, string>& parameters,
                                              Datum default_number,
                                              GenericDataType datatype){
    
    try {
        return getNumberForAttribute(attribute, parameters, datatype);
    } catch (MissingAttributeException){
        return default_number;
    }
}



boost::filesystem::path ComponentRegistry::getPath(std::string working_path,
													std::string expression){

	return expandPath(working_path, expression);
}

void ComponentRegistry::dumpToStdErr(){

    unordered_map< string, shared_ptr<Component> >::iterator i;
    for(i = instances.begin(); i != instances.end(); ++i){
        pair< string, shared_ptr<Component> > instance = *i;
        cerr << instance.first << ": " << instance.second.get() << endl;
    }
    
}


END_NAMESPACE_MW






















