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
    ReadOnlyVariable(props),
    selected_index(NO_SELECTION),
    advanceOnAccept(false)
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
        // Issue an error message only if the experiment is running.  Otherwise, all selection variable indexing
        // expressions will produce load-time errors (since ParsedExpressionVariable's constructors evaluate the
        // expression to test for validity).
        if (StateSystem::instance(false) && StateSystem::instance()->isRunning()) {
            merror(M_PARADIGM_MESSAGE_DOMAIN, "Selection variable has no tentative selections available.  Returning 0.");
        }
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
			
			merror(M_PARADIGM_MESSAGE_DOMAIN, "%s", e.what());
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


shared_ptr<mw::Component> SelectionVariableFactory::createObject(std::map<std::string, std::string> parameters,
														ComponentRegistry *reg) {
	REQUIRE_ATTRIBUTES(parameters,
					   "tag",
					   "values",
					   "sampling_method",
					   "nsamples",
					   "selection");
	
	Datum defaultValue(0L); // the default value Datum object.
	std::string groups(EXPERIMENT_DEFINED_VARIABLES);
    bool advanceOnAccept = false;
	
	string tag(parameters.find("tag")->second);
	
	if(parameters.find("groups") != parameters.end()) {
        groups.append(", ");
        groups.append(parameters.find("groups")->second);
	}
	
	if (parameters.find("advance_on_accept") != parameters.end()) {
		try {
			advanceOnAccept = reg->getBoolean(parameters.find("advance_on_accept")->second);
		} catch (boost::bad_lexical_cast &) {
			throw InvalidAttributeException(parameters["reference_id"], "advance_on_accept", parameters.find("advance_on_accept")->second);
		}
	}
	
	// TODO when the variable properties get fixed, we can get rid of this nonsense
    VariableProperties props(defaultValue,
                             tag,
                             M_WHEN_CHANGED,
                             false,
                             groups);
	
	boost::shared_ptr<SelectionVariable>selectionVar;
	selectionVar = global_variable_registry->createSelectionVariable(&props);
    
    selectionVar->setAdvanceOnAccept(advanceOnAccept);
	
	// get the values
    std::vector<Datum> values;
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
    } else if(to_lower_copy(parameters.find("selection")->second) == "sequential") {
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
	
	for(std::vector<Datum>::const_iterator i = values.begin();
		i != values.end();
		++i) {
		selectionVar->addValue(*i);
	}
	
	return selectionVar;
}


END_NAMESPACE_MW


























