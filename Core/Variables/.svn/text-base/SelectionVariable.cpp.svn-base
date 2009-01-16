/*
 *  SelectionVariable.cpp
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 3/1/06.
 *  Copyright 2006 MIT. All rights reserved.
 *
 */

#include "SelectionVariable.h"
#include "VariableReference.h"
#include "VariableProperties.h"
#include "VariableRegistry.h"
#include "SequentialSelection.h"
#include "RandomWORSelection.h"
#include "RandomWithReplacementSelection.h"
#include "ComponentRegistry_new.h"
#include <vector>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/spirit/utility/confix.hpp>
#include <boost/spirit/utility/lists.hpp>
using namespace mw;


SelectionVariable::SelectionVariable(VariableProperties *props) : 
Selectable(), 
Variable(props){
}

SelectionVariable::SelectionVariable(VariableProperties *props, 
									   shared_ptr<Selection> _selection) : 
Selectable(),
Variable(props){
	attachSelection(_selection);
}




void SelectionVariable::nextValue(){
	int index;
	
	if(selection != NULL){
		
		try {
			index = selection->draw();
			
		} catch (std::exception &e){
			
			merror(M_PARADIGM_MESSAGE_DOMAIN, e.what());
			return;
		}
		
		selected_value = values[index];
		
		if(selected_value != NULL){
		
			// announce your new value so that the event stream contains
			// all information about what happened in the experiment
			announce();
			performNotifications(selected_value->getValue());
		}
		
	} else {
		merror(M_PARADIGM_MESSAGE_DOMAIN,
			   "Attempt to advance a selection variable with a NULL selection");
	}
}


Data SelectionVariable::getValue(){
	if(selected_value == NULL){
		nextValue();
	}
	
	if(selected_value == NULL){
		merror(M_PARADIGM_MESSAGE_DOMAIN,
			   "Attempt to select a value from a selection variable with no values defined");
		return Data(0L);
	}
	
	
	return selected_value->getValue();
}

Variable *SelectionVariable::clone(){
	// This isn't quite right, but we can run with it for now
	return new VariableReference(this);
}

shared_ptr<mw::Component> SelectionVariableFactory::createObject(std::map<std::string, std::string> parameters,
														mwComponentRegistry *reg) {
	REQUIRE_ATTRIBUTES(parameters,
					   "tag",
					   "values",
					   "sampling_method",
					   "nsamples",
					   "selection");
	
	std::string full_name("");
	std::string description("");
	GenericDataType type = M_INTEGER;
	WhenType editable = M_ALWAYS; // when can we edit the variable
	bool viewable = true; // can the user see this variable
	bool persistant = false; // save the variable from run to run
	WhenType logging = M_WHEN_CHANGED; // when does this variable get logged
	Data defaultValue(0L); // the default value Data object.	
	std::string groups("");
	
	string tag(parameters.find("tag")->second);
	
	if(parameters.find("full_name") != parameters.end()) {
		full_name = parameters.find("full_name")->second;
	}
	
	if(parameters.find("description") != parameters.end()) {
		description = parameters.find("description")->second;
	}
	
	string type_lower = to_lower_copy(parameters.find("type")->second);
	
	if(type_lower == "integer") {
		type = M_INTEGER;
		defaultValue = 0L;
	} else if(type_lower == "float" || 
			  type_lower == "double") {
		type = M_FLOAT;
		defaultValue = 0.0;
	} else if(type_lower == "string") {
		type = M_STRING;
		defaultValue = "";
	} else if(type_lower == "boolean") {
		type = M_BOOLEAN;
		defaultValue = false;
	} else if(type_lower == "selection"){
		type = M_UNDEFINED;
		defaultValue = 0L;
	} else {
		throw InvalidAttributeException(parameters.find("reference_id")->second, "type", parameters.find("type")->second);
	}
	
	
	// TODO...when changed?
	
	if(parameters.find("editable") != parameters.end()){
		string editable_lower = to_lower_copy(parameters.find("editable")->second);
		if(editable_lower == "never") {
			editable = M_NEVER;
		} else if(editable_lower == "when_idle") {
			editable = M_WHEN_IDLE;
		} else if(editable_lower == "always") {
			editable = M_ALWAYS;
		} else if(editable_lower == "at_startup") {
			editable = M_AT_STARTUP;
		} else if(editable_lower == "every_trial") {
			editable = M_EVERY_TRIAL;
		} else if(editable_lower == "when_changed") {
			editable = M_WHEN_CHANGED;
		} else {
			throw InvalidAttributeException(parameters.find("reference_id")->second, "editable", parameters.find("editable")->second);
		}
	} else {
		editable = M_NEVER;
	}
	
	if(parameters.find("viewable") != parameters.end()){
		try {
			viewable = reg->getBoolean(parameters.find("viewable")->second);
		} catch (boost::bad_lexical_cast &) {
			throw InvalidAttributeException(parameters.find("reference_id")->second, "viewable", parameters.find("viewable")->second);
		}
	} else {
		viewable = true;
	}
	
	
	if(parameters.find("persistant") != parameters.end()){
		try {
			persistant = boost::lexical_cast<bool>(parameters.find("persistant")->second);
		} catch (boost::bad_lexical_cast &) {
			throw InvalidAttributeException(parameters.find("reference_id")->second, "persistant", parameters.find("persistant")->second);
		}
	} else {
		persistant = false;
	}
	
	if(parameters.find("logging") != parameters.end()){
		
		string logging_lower = to_lower_copy(parameters.find("logging")->second);
		if(logging_lower == "never") {
			logging = M_NEVER;
		} else if(logging_lower == "when_idle") {
			logging = M_WHEN_IDLE;
		} else if(logging_lower == "always") {
			logging = M_ALWAYS;
		} else if(logging_lower == "at_startup") {
			logging = M_AT_STARTUP;
		} else if(logging_lower == "every_trial") {
			logging = M_EVERY_TRIAL;
		} else if(logging_lower == "when_changed") {
			logging = M_WHEN_CHANGED;
		} else {
			throw InvalidAttributeException(parameters.find("reference_id")->second, "logging", parameters.find("logging")->second);
		}
	} else {
		logging = M_ALWAYS;
	}
	
/*	switch(type) {
		case M_INTEGER:
		case M_FLOAT:
		case M_BOOLEAN:
			try {
				defaultValue = Data(type, boost::lexical_cast<float>(parameters.find("default_value")->second));					
			} catch (bad_lexical_cast &) {					
				throw InvalidAttributeException("default_value", parameters.find("default_value")->second);
			}
			break;
		case M_STRING:
			defaultValue = Data(parameters.find("default_value")->second);
			break;
		default:
			throw InvalidAttributeException("default_value", parameters.find("default_value")->second);
			break;
	}*/
	
	if(parameters.find("groups") != parameters.end()) {
		groups = parameters.find("groups")->second;
	}
	
	// TODO when the variable properties get fixed, we can get rid of this nonsense
	VariableProperties props(&defaultValue,
							  tag,
							  full_name,
							  description,
							  M_ALWAYS,
							  M_WHEN_CHANGED,
							  true,
							  false,
							  M_INTEGER_INFINITE,
							  std::string(""));
	
	boost::shared_ptr<SelectionVariable>selectionVar;
	selectionVar = GlobalVariableRegistry->createSelectionVariable(&props);
	
	// get the values
	std::vector<std::string> values;

	boost::tokenizer<> tok(parameters["values"]);
	for(tokenizer<>::iterator beg=tok.begin(); beg!=tok.end();++beg){
		values.push_back(*beg);
    }
	
	
	// get the sampling method
	std::map<std::string, std::string>::const_iterator samplingMethodElement = parameters.find("sampling_method");
	if(samplingMethodElement == parameters.end()) {
		throw;	
	}
	
	// get the number of samples
	unsigned int numSamples = 0;
	try {
		numSamples = boost::lexical_cast< unsigned int >( parameters.find("nsamples")->second );
	} catch (bad_lexical_cast &) {
		throw InvalidAttributeException(parameters.find("reference_id")->second, "nsamples", parameters.find("nsamples")->second);			
	}
	
	// if it's cycles, multiply by the number of elements in the possible values
	if(to_lower_copy(parameters.find("sampling_method")->second) == "cycles") {
		numSamples *= values.size();
	} else if(to_lower_copy(parameters.find("sampling_method")->second) == "samples") {
		// do nothing
	} else {
		throw InvalidAttributeException(parameters.find("reference_id")->second, "sampling_method", parameters.find("sampling_method")->second);
	}
	
	// get the selection type
	shared_ptr<Selection> selection;
	if(to_lower_copy(parameters.find("selection")->second) == "sequential_ascending") {
		selection = shared_ptr<SequentialSelection>(new SequentialSelection(numSamples, true));
	} else if(to_lower_copy(parameters.find("selection")->second) == "sequential_descending") {
		selection = shared_ptr<SequentialSelection>(new SequentialSelection(numSamples, false));			
	} else if(to_lower_copy(parameters.find("selection")->second) == "random_without_replacement") {
		selection = shared_ptr<RandomWORSelection>(new RandomWORSelection(numSamples));			
	} else if(to_lower_copy(parameters.find("selection")->second) == "random_with_replacement") {
		selection = shared_ptr<RandomWithReplacementSelection>(new RandomWithReplacementSelection(numSamples));			
	} else {
		throw InvalidAttributeException(parameters.find("reference_id")->second, "selection", parameters.find("selection")->second);
	}
	
	selectionVar->attachSelection(selection);
	
	for(std::vector<std::string>::const_iterator i = values.begin();
		i != values.end();
		++i) {
		shared_ptr<Variable> valueVariable = reg->getVariable(*i);
		selectionVar->addValue(valueVariable);
	}
	
	return selectionVar;
}

