/*
 *  SelectionVariable.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 3/1/06.
 *  Copyright 2006 MIT. All rights reserved.
 *
 */

#include "SelectionVariable.h"

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/case_conv.hpp>

#include "VariableReference.h"
#include "VariableProperties.h"
#include "VariableRegistry.h"
#include "SequentialSelection.h"
#include "RandomWORSelection.h"
#include "RandomWithReplacementSelection.h"
#include "ComponentRegistry.h"
#include "ExpressionVariable.h"

using boost::algorithm::to_lower_copy;


BEGIN_NAMESPACE_MW


SelectionVariable::SelectionVariable(VariableProperties *props, shared_ptr<Selection> _selection) :
    Selectable(),
    Variable(props),
    selected_index(NO_SELECTION)
{
    if (_selection) {
        attachSelection(_selection);
    }
}


Datum SelectionVariable::getTentativeSelection(int index) {
    if (!selection) {
        merror(M_PARADIGM_MESSAGE_DOMAIN, "Internal error: selection variable has no selection attached");
        return Datum(0L);
    }
    
    const std::vector<int>& tenativeSelections = selection->getTentativeSelections();
    
    if (tenativeSelections.size() == 0) {
        merror(M_PARADIGM_MESSAGE_DOMAIN, "Selection variable has no tentative selections available.  Returning 0.");
        return Datum(0L);
    }
    
    if ((index < 0) || (index >= tenativeSelections.size())) {
        merror(M_PARADIGM_MESSAGE_DOMAIN, "Selection variable index (%d) is out of bounds.  Returning 0.", index);
        return Datum(0L);
    }
        
    return values[tenativeSelections[index]];
}


void SelectionVariable::nextValue() {
	if (selection != NULL) {
		
		try {
            
			selected_index = selection->draw();

		} catch (std::exception &e) {
			
			merror(M_PARADIGM_MESSAGE_DOMAIN, e.what());
			return;
            
		}
		
        // announce your new value so that the event stream contains
        // all information about what happened in the experiment
        announce();
        performNotifications(values[selected_index]);
		
	} else {
		merror(M_PARADIGM_MESSAGE_DOMAIN,
			   "Attempt to advance a selection variable with a NULL selection");
	}
}


Datum SelectionVariable::getValue() {
	if (selected_index == NO_SELECTION) {
		nextValue();
	}
	
	if (selected_index == NO_SELECTION) {
		merror(M_PARADIGM_MESSAGE_DOMAIN,
			   "Attempt to select a value from a selection variable with no values defined");
		return Datum(0L);
	}
	
	return values[selected_index];
}


Variable *SelectionVariable::clone(){
	// This isn't quite right, but we can run with it for now
	return new VariableReference(this);
}


shared_ptr<mw::Component> SelectionVariableFactory::createObject(std::map<std::string, std::string> parameters,
														ComponentRegistry *reg) {
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
 Datum defaultValue(0L); // the default value Datum object.	
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
		throw InvalidAttributeException(parameters["reference_id"], "type", parameters.find("type")->second);
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
			throw InvalidAttributeException(parameters["reference_id"], "editable", parameters.find("editable")->second);
		}
	} else {
		editable = M_NEVER;
	}
	
	if(parameters.find("viewable") != parameters.end()){
		try {
			viewable = reg->getBoolean(parameters.find("viewable")->second);
		} catch (boost::bad_lexical_cast &) {
			throw InvalidAttributeException(parameters["reference_id"], "viewable", parameters.find("viewable")->second);
		}
	} else {
		viewable = true;
	}
	
	
	if(parameters.find("persistant") != parameters.end()){
		try {
			persistant = boost::lexical_cast<bool>(parameters.find("persistant")->second);
		} catch (boost::bad_lexical_cast &) {
			throw InvalidAttributeException(parameters["reference_id"], "persistant", parameters.find("persistant")->second);
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
			throw InvalidAttributeException(parameters["reference_id"], "logging", parameters.find("logging")->second);
		}
	} else {
		logging = M_ALWAYS;
	}
	
/*	switch(type) {
		case M_INTEGER:
		case M_FLOAT:
		case M_BOOLEAN:
			try {
				defaultValue = Datum(type, boost::lexical_cast<float>(parameters.find("default_value")->second));					
			} catch (bad_lexical_cast &) {					
				throw InvalidAttributeException("default_value", parameters.find("default_value")->second);
			}
			break;
		case M_STRING:
			defaultValue = Datum(parameters.find("default_value")->second);
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
	selectionVar = global_variable_registry->createSelectionVariable(&props);
	
	// get the values
    std::vector<stx::AnyScalar> values;
    ParsedExpressionVariable::evaluateExpressionList(parameters["values"], values);
	
	// get the sampling method
	std::map<std::string, std::string>::const_iterator samplingMethodElement = parameters.find("sampling_method");
	if(samplingMethodElement == parameters.end()) {
		throw;	
	}
	
	// get the number of samples
	unsigned int numSamples = 0;
	try {
		numSamples = boost::lexical_cast< unsigned int >( parameters.find("nsamples")->second );
	} catch (boost::bad_lexical_cast &) {
		throw InvalidAttributeException(parameters["reference_id"], "nsamples", parameters.find("nsamples")->second);			
	}
	
	// if it's cycles, multiply by the number of elements in the possible values
	if(to_lower_copy(parameters.find("sampling_method")->second) == "cycles") {
		numSamples *= values.size();
	} else if(to_lower_copy(parameters.find("sampling_method")->second) == "samples") {
		// do nothing
	} else {
		throw InvalidAttributeException(parameters["reference_id"], "sampling_method", parameters.find("sampling_method")->second);
	}
	
    bool autoreset_behavior = false;
    string autoreset_value = parameters["autoreset"];
    
    if(!autoreset_value.empty()){ 
        boost::algorithm::to_lower(autoreset_value);
        if(autoreset_value == "yes" || autoreset_value == "1" || autoreset_value == "true"){
            autoreset_behavior = true;
        }
    }
    
	// get the selection type
	shared_ptr<Selection> selection;
	if(to_lower_copy(parameters.find("selection")->second) == "sequential_ascending") {
		selection = shared_ptr<SequentialSelection>(new SequentialSelection(numSamples, true, autoreset_behavior));
	} else if(to_lower_copy(parameters.find("selection")->second) == "sequential_descending") {
		selection = shared_ptr<SequentialSelection>(new SequentialSelection(numSamples, false, autoreset_behavior));			
	} else if(to_lower_copy(parameters.find("selection")->second) == "random_without_replacement") {
		selection = shared_ptr<RandomWORSelection>(new RandomWORSelection(numSamples, autoreset_behavior));			
	} else if(to_lower_copy(parameters.find("selection")->second) == "random_with_replacement") {
		selection = shared_ptr<RandomWithReplacementSelection>(new RandomWithReplacementSelection(numSamples, autoreset_behavior));			
	} else {
		throw InvalidAttributeException(parameters["reference_id"], "selection", parameters.find("selection")->second);
	}
	
	selectionVar->attachSelection(selection);
	
	for(std::vector<stx::AnyScalar>::const_iterator i = values.begin();
		i != values.end();
		++i) {
		selectionVar->addValue(Datum(*i));
	}
	
	return selectionVar;
}


END_NAMESPACE_MW


























