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
#include "BlockAndProtocol.h"
#include "States.h"
#include "TrialBuildingBlocks.h"
#include "DynamicStimulusActions.h"
#include "ScheduledActions.h"
#include "PulseAction.h"
#include "StandardSounds.h"
#include "SoundGroup.hpp"
#include "Averagers.h"
#include "SimpleStaircase.h"
#include "DataFileDevice.hpp"
#include "DummyIODevice.h"
#include "LegacyIODevice.h"
#include "StimulusDisplayDevice.h"
#include "XMLParser.h"
#include "BiasMonitor.h"
#include "StandardComponentFactory.h"
#include "StandardStimulusFactory.h"
#include "StandardStimuli.h"


BEGIN_NAMESPACE_MW


shared_ptr<ComponentRegistry> ComponentRegistry::shared_component_registry;

ComponentRegistry::ComponentRegistry() {

	// register all of the built-ins here
	registerFactory("experiment", new ExperimentFactory());
    
    registerFactory<StandardComponentFactory, Protocol>();
    registerFactory<StandardComponentFactory, Block>();
    registerFactory<StandardComponentFactory, GenericListState>();
    registerFactory<StandardComponentFactory, Trial>();
    registerFactory<StandardComponentFactory, TaskSystem>();
    registerFactory<StandardComponentFactory, TaskSystemState>();
	
	registerFactory("variable", boost::make_shared<VariableFactory>());
	registerFactory("variable/selection", boost::make_shared<SelectionVariableFactory>());
	
	
    registerFactory<StandardComponentFactory, ScheduledActions>();
    registerFactory<StandardComponentFactory, If>();
    registerFactoryAlias<If>("action/when");
    registerFactory<StandardComponentFactory, Else>();
    registerFactoryAlias<Else>("action/otherwise");
    registerFactory<StandardComponentFactory, IfElse>();
    registerFactoryAlias<IfElse>("action/choose");
    registerFactory<StandardComponentFactory, While>();
	
	
    registerFactory<StandardComponentFactory, Assignment>();
    registerFactory<StandardComponentFactory, Pulse>();
    registerFactory<StandardComponentFactory, ReportString>();
    registerFactory<StandardComponentFactory, AssertionAction>();
	registerFactory("action/next_selection", new NextVariableSelectionFactory());
	registerFactory("action/set_timebase", new SetTimeBaseFactory());
    registerFactory<StandardComponentFactory, StartTimer>();
    registerFactory<StandardComponentFactory, Wait>();
    registerFactory<StandardComponentFactory, WaitForCondition>();
	registerFactory("action/load_stimulus", new LoadStimulusFactory());
    registerFactory("action/unload_stimulus", new UnloadStimulusFactory());
    registerFactory("action/queue_stimulus", new QueueStimulusFactory());
	registerFactoryAlias("action/queue_stimulus", "action/show_stimulus");
	registerFactory("action/live_queue_stimulus", new LiveQueueStimulusFactory());
	registerFactory("action/dequeue_stimulus", new DequeueStimulusFactory());
	registerFactoryAlias("action/dequeue_stimulus", "action/hide_stimulus");
	registerFactory("action/bring_stimulus_to_front", new BringStimulusToFrontFactory());
	registerFactory("action/send_stimulus_to_back", new SendStimulusToBackFactory());
    registerFactory<StandardComponentFactory, UpdateStimulusDisplay>();
    registerFactoryAlias<UpdateStimulusDisplay>("action/update_display");
    registerFactory<StandardComponentFactory, ClearStimulusDisplay>();
    registerFactoryAlias<ClearStimulusDisplay>("action/clear_display");
    registerFactory<StandardComponentFactory, LoadSound>();
    registerFactory<StandardComponentFactory, UnloadSound>();
    registerFactory<StandardComponentFactory, PlaySound>();
    registerFactory<StandardComponentFactory, PauseSound>();
    registerFactory<StandardComponentFactory, StopSound>();
	registerFactory("action/start_device_io", new StartDeviceIOFactory());
    registerFactoryAlias("action/start_device_io", "action/start_io_device");
	registerFactory("action/stop_device_io", new StopDeviceIOFactory());
    registerFactoryAlias("action/stop_device_io", "action/stop_io_device");
	registerFactory("action/reset_selection", new ResetSelectionFactory());
	registerFactory("action/accept_selections", new AcceptSelectionsFactory());
	registerFactory("action/reject_selections", new RejectSelectionsFactory());
    registerFactory<StandardComponentFactory, StopExperiment>();
    registerFactory<StandardComponentFactory, PauseExperiment>();
    registerFactory<StandardComponentFactory, ResumeExperiment>();
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
    registerFactory<StandardComponentFactory, EyeStatusMonitorVer1>();
	registerFactory("filter/boxcar_filter_1d", new Filter_BoxcarFilter1DFactory());
	registerFactory("linear_filter_1d", new Filter_LinearFilter1DFactory());
	registerFactory("staircase", new SimpleStaircaseOptimizerFactory());
    registerFactory("bias_monitor", new BiasMonitorFactory());
	
	// IO devices
    registerFactory<StandardComponentFactory, DataFileDevice>();
    registerFactory<StandardComponentFactory, DummyIODevice>();
    registerFactory<StandardComponentFactory, IOChannelRequest>();
    registerFactory<StandardComponentFactory, StimulusDisplayDevice>();
	
	// stimuli
    registerStandardStimuli(*this);
    registerFactory<StandardComponentFactory, StimulusGroup>();
	
	// sounds
    registerStandardSounds(*this);
    registerFactory<StandardComponentFactory, SoundGroup>();
    
}


// Factory-oriented methods


void ComponentRegistry::registerFactory(const std::string &type_name, shared_ptr<ComponentFactory> factory) {
    shared_ptr<ComponentFactory> existing_factory = factories[type_name];
    if(existing_factory != NULL){
        throw ComponentFactoryConflictException(type_name);
	}
    
    factories[type_name] = factory;
}


shared_ptr<ComponentFactory> ComponentRegistry::findFactory(const std::string &type_name) {
	shared_ptr<ComponentFactory> factory(factories[type_name]);
	
	if (!factory) {
		// try splitting
		std::vector<std::string> split_vector;
		split(split_vector, type_name, boost::algorithm::is_any_of("/"));
		factory = factories[split_vector[0]];
	}
	
	return factory;
}


bool ComponentRegistry::hasFactory(const std::string &type_name) {
    return bool(findFactory(type_name));
}


shared_ptr<ComponentFactory> ComponentRegistry::getFactory(const std::string &type_name) {
    shared_ptr<ComponentFactory> factory = findFactory(type_name);
	
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


void ComponentRegistry::registerObject(std::string tag_name, shared_ptr<mw::Component> component) {
	if(tag_name.empty()){
		throw SimpleException("Attempt to register a component with an empty name");
	}
	if(component == NULL){
		throw SimpleException("Attempt to register empty component", tag_name);
	}
    
    component->setTag(tag_name);
    
    // If the tag name is already registered
    if(instances.find(tag_name) != instances.end() && instances[tag_name] != NULL){
        
        shared_ptr<Component> preexisting = instances[tag_name];
        
        if(preexisting == NULL){
            throw SimpleException("Attempt to redefine an existing component.  All names within an experiment must be unique", tag_name);
        }

        shared_ptr<AmbiguousComponentReference> ambiguous_component;
        if(preexisting->isAmbiguous()){
            ambiguous_component = boost::dynamic_pointer_cast<AmbiguousComponentReference>(preexisting);
        } else {
            ambiguous_component = shared_ptr<AmbiguousComponentReference>(new AmbiguousComponentReference());
            ambiguous_component->addAmbiguousComponent(preexisting);
        }
        
        ambiguous_component->addAmbiguousComponent(component);
        
        instances[tag_name] = boost::dynamic_pointer_cast<AmbiguousComponentReference, Component>(ambiguous_component);
        
        return;
    }
    
	instances[tag_name] = component;
	tagnames_by_id[component->getCompactID()] = tag_name;
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
    if (tag_name.empty()) {
        // It's valid for a stimulus to have no tag (e.g. it could be a member of a stimulus group that is only
        // accessed by index)
        return;
    }
    if (!stimNode){
        throw SimpleException("Attempt to register empty stimulus node", tag_name);
    }
    auto &slot = stimulus_nodes[tag_name];
    if (slot) {
        throw SimpleException(boost::format("A stimulus with the name \"%s\" already exists") % tag_name);
    }
    slot = stimNode;
}


// Instance lookups with some extra parsing smarts
boost::shared_ptr<Variable> ComponentRegistry::getVariable(std::string expression) {
    auto &var = variable_cache[expression];  // Take a reference, so that we'll add to the cache if needed
    if (!var) {
        // Not in the cache, so check the variable registry
        var = global_variable_registry->getVariable(boost::algorithm::trim_copy(expression));
        if (!var) {
            // Not in the variable registry, so try the expression parser, which will throw
            // UnknownVariableException if the expression contains a bad variable name
            var = boost::make_shared<ParsedExpressionVariable>(expression);
        }
    }
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


boost::shared_ptr<Variable> ComponentRegistry::getParsedString(std::string input) {
    auto &str = parsed_string_cache[input];  // Take a reference, so that we'll add to the cache if needed
    if (!str) {
        // Not in the cache, so try the expression parser.  This will throw UnknownVariableException
        // if the expression contains an interpolated variable with a bad variable name.
        str = boost::make_shared<ParsedExpressionVariable>(ParsedExpressionVariable::parseUnquotedStringLiteral(input));
    }
    return str;
}


shared_ptr<mw::StimulusNode>	ComponentRegistry::getStimulus(std::string expression){
	
	// regex for parsing the stimulus string
    boost::regex stimulus_regex("(.+?)(\\[(.+)\\])?");
	
    boost::smatch matches;
	try{
		
		regex_match(expression, matches, stimulus_regex); 
		// matches[0] contains the whole string 
		// matches[1] contains the stimulus "stem" 
		// matches[2] contains the index expression (if there is one), 
		//			  including brackets. 
		// matches[3] contains the index expression (if there is one), 
		//			  without brackets.
		
   } catch (boost::regex_error& e) {
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
		
		shared_ptr<mw::Component> newStimulusNodeGroup = boost::dynamic_pointer_cast<mw::Component, StimulusNodeGroup>(stimulus_node_group);
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
	boost::regex multi_dimensional_group_regex(".+\\]\\s*\\[.+");
	if( regex_match(index_pattern, multi_dimensional_group_regex) ){
		
		// If it's multidimensional, we need to split up the contents of the
		// outer brackets and multiply/add them into a c-style multidimensional
		// array index. 

		// We're going to use this string stream for syntactic convenience in
		// concatenating things together.
		std::ostringstream index_expression_stream;
		index_expression_stream << "0";  // get things started
		
		
		boost::regex index_regex("\\]\\s*\\[");

		// split the index strings
        boost::sregex_token_iterator i_t(index_pattern.begin(),
									   index_pattern.end(),
									   index_regex, -1);
        boost::sregex_token_iterator j_t;
		
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


// The following enables the use of GenericDataType values in data_cache keys.  See
// http://www.boost.org/doc/libs/1_51_0/doc/html/hash/custom.html for details.
std::size_t hash_value(const GenericDataType &type) {
    boost::hash<int> hasher;
    return hasher(type);
}


Datum ComponentRegistry::getValue(std::string expression, GenericDataType type) {

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
            doubleValue = boost::lexical_cast<double>(expression);
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
          // No lexical_cast for other types
          break;
    }
    
    if (!value.isUndefined()) {
        data_cache[cacheKey] = shared_ptr<Datum>(new Datum(value));
        return value;
    }
  } catch (NonFatalParserException& e){
      // Until we work out how to effectively flag these issues, treat them as fatal errors
      throw FatalParserException(e.what());
  } catch (boost::bad_lexical_cast& e){
    // no biggie, we can do this the hard(er) way
  }
  
  
	return Datum(ParsedExpressionVariable::evaluateExpression(expression));
}


Datum ComponentRegistry::getNumber(std::string expression, GenericDataType type){

  Datum value;
    switch(type){
      case M_FLOAT:
      case M_INTEGER:
      case M_BOOLEAN:
      case M_STRING:
          value = getValue(expression, type);
          if (value.getDataType() == type) {
              return value;
          }
          break;
      default:
          throw FatalParserException("Attempt to cast a number of invalid type");
    }
	
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
			return value;			
	}
}


boost::filesystem::path ComponentRegistry::getPath(std::string working_path,
													std::string expression){

	return expandPath(working_path, expression);
}

void ComponentRegistry::dumpToStdErr(){

    boost::unordered_map< string, shared_ptr<Component> >::iterator i;
    for(i = instances.begin(); i != instances.end(); ++i){
        pair< string, shared_ptr<Component> > instance = *i;
        cerr << instance.first << ": " << instance.second.get() << endl;
    }
    
}


END_NAMESPACE_MW






















