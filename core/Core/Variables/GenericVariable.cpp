/**
 * GenericVariable.cpp
 *
 * History:
 * David Cox on Tue Dec 10 2002 - Created.
 * Paul Jankunas on 01/27/05 - Fixed spacing added codec code implementation.
 *
 * Copyright (c) 2002 MIT. All rights reserved.
 */

#include "GenericVariable.h"
#include "ExpressionVariable.h"
#include "GlobalVariable.h"
#include "ScopedVariable.h"
#include "Experiment.h"
#include "Event.h"
#include "EventBuffer.h"
#include <boost/algorithm/string/case_conv.hpp>
#include "ComponentRegistry.h"
#include "EventBuffer.h"
#include "TrialBuildingBlocks.h"

using boost::algorithm::to_lower_copy;


BEGIN_NAMESPACE_MW


/*******************************************************************
*                   Variable member functions
*******************************************************************/


MWTime Variable::getCurrentTimeUS() {
    auto clock = Clock::instance(false);
    if (clock) {
        return clock->getCurrentTimeUS();
    }
    return 0;
}


Variable::Variable(const VariableProperties &properties) :
    properties(properties),
    codec_code(-1),
    logging(properties.getLogging()),
    event_target(global_outgoing_event_buffer)
{
    setTag(properties.getTagName());
}


// Factory method
shared_ptr<mw::Component> VariableFactory::createObject(std::map<std::string, std::string> parameters,
											   ComponentRegistry *reg){
	REQUIRE_ATTRIBUTES(parameters,
					   "tag",
					   "default_value");
	std::string tag;
	std::string type_string("");
	std::string persistant_string("");
    std::string exclude_from_data_file_string("");
	std::string logging_string("");
	std::string scope_string("");
	
	GenericDataType type = M_UNDEFINED;
	bool persistant = false; // save the variable from run to run
    bool excludeFromDataFile = false; // should the variable be excluded from data files
	WhenType logging = M_WHEN_CHANGED; // when does this variable get logged
	Datum defaultValue(0L); // the default value Datum object.
	std::string groups(EXPERIMENT_DEFINED_VARIABLES);
	
	GET_ATTRIBUTE(parameters, tag, "tag", "NO_TAG");
	GET_ATTRIBUTE(parameters, type_string, "type", "any");
	GET_ATTRIBUTE(parameters, persistant_string, "persistant", "0");
    GET_ATTRIBUTE(parameters, exclude_from_data_file_string, "exclude_from_data_file", "0");
	GET_ATTRIBUTE(parameters, logging_string, "logging", "when_changed");
	GET_ATTRIBUTE(parameters, scope_string, "scope", "global");


	type_string = to_lower_copy(type_string);
	
    if (type_string.empty() || type_string == "any") {
        type = M_UNDEFINED;
    } else if(type_string == "integer") {
		type = M_INTEGER;
		defaultValue = 0L;
	} else if(type_string == "float" || 
			  type_string == "double") {
		type = M_FLOAT;
		defaultValue = 0.0;
	} else if(type_string == "string") {
		type = M_STRING;
		defaultValue = "";
	} else if(type_string == "boolean") {
		type = M_BOOLEAN;
		defaultValue = false;
	} else if(type_string == "list") {
		type = M_LIST;
		defaultValue = Datum(M_LIST, 0);
	} else {
		throw InvalidAttributeException(parameters["reference_id"], "type", parameters.find("type")->second);
	}
	
	try {
		persistant = reg->getBoolean(persistant_string);
	} catch (boost::bad_lexical_cast &) {
		throw InvalidAttributeException(parameters["reference_id"], "persistant", parameters.find("persistant")->second);
	}
    
    try {
        excludeFromDataFile = reg->getBoolean(exclude_from_data_file_string);
    } catch (boost::bad_lexical_cast &) {
        throw InvalidAttributeException(parameters["reference_id"], "exclude_from_data_file", parameters.find("exclude_from_data_file")->second);
    }
	
	
	if(to_lower_copy(logging_string) == "never") {
		logging = M_NEVER;
	} else if(to_lower_copy(logging_string) == "when_changed") {
		logging = M_WHEN_CHANGED;
	} else {
		throw InvalidAttributeException(parameters["reference_id"], "logging", logging_string);
	}
	
	
	
	switch(type) {
        case M_UNDEFINED:
            // Type is "any", so let the expression parser infer the type
            defaultValue = ParsedExpressionVariable::evaluateExpression(parameters.find("default_value")->second);
            break;
		case M_INTEGER:
		case M_FLOAT:
		case M_BOOLEAN:
			try {
				defaultValue = reg->getNumber(parameters.find("default_value")->second, type);
				//mData(type, boost::lexical_cast<double>(parameters.find("default_value")->second));					
			} catch (boost::bad_lexical_cast &) {
				throw InvalidAttributeException(parameters["reference_id"], "default_value", parameters.find("default_value")->second);
			}
			break;
		case M_STRING:
			defaultValue = Datum(parameters.find("default_value")->second);
			break;
        case M_LIST: {
            std::vector<Datum> values;
            ParsedExpressionVariable::evaluateExpressionList(parameters.find("default_value")->second, values);
            
            defaultValue = Datum(M_LIST, int(values.size()));
            for (int i = 0; i < values.size(); i++) {
                defaultValue.setElement(i, values[i]);
            }
            
            break;
        }
		default:
			throw InvalidAttributeException(parameters["reference_id"], "default_value", parameters.find("default_value")->second);
			break;
	}
	
	if(parameters.find("groups") != parameters.end()) {
        groups.append(", ");
		groups.append(parameters.find("groups")->second);
	}
	
	// TODO when the variable properties get fixed, we can get rid of this nonsense
    VariableProperties props(defaultValue,
                             tag,
                             logging,
                             persistant,
                             groups,
                             excludeFromDataFile);
	
	shared_ptr<mw::Component>newVar;
	
	scope_string = to_lower_copy(scope_string);
	if(scope_string == "global") {
		newVar = global_variable_registry->createGlobalVariable(props);
	} else if(scope_string == "local") {
		newVar = global_variable_registry->createScopedVariable(GlobalCurrentExperiment, props);
	} else {
		throw InvalidAttributeException(parameters["reference_id"], "scope", parameters.find("scope")->second);
	}
	
	return newVar;
}


void Variable::addChild(std::map<std::string, std::string> parameters,
						 ComponentRegistry *reg,
						 shared_ptr<mw::Component> child) {
	
	shared_ptr<Action> act = boost::dynamic_pointer_cast<Action,mw::Component>(child);
	
	if(act == 0) {
		throw SimpleException("Attempting to add illegal object (" + child->getTag() + ") to variable (" + this->getVariableName() + ")");
	}
	
	
	shared_ptr<ActionVariableNotification> avn(new ActionVariableNotification(act));
	addNotification(avn);	
}

void Variable::addNotification(const shared_ptr<VariableNotification> &_notif) {
	if (!_notif) {
		mwarning(M_PARADIGM_MESSAGE_DOMAIN,
					"Attempt to add a null notification to a variable");
		return;
	}
	notifications.addToBack(_notif);
}


void Variable::performNotifications(Datum data, MWTime timeUS) {
    Locker locker(notifications);
    
    auto node = notifications.getFrontmost();
    
    while (node) {
        node->notify(data, timeUS);
        node = node->getNext();
    }
}


void Variable::announce(MWTime timeUS){
    if (properties.getLogging() == M_WHEN_CHANGED && event_target != 0) {
        shared_ptr<Event> new_event(new Event(codec_code, timeUS, getValue()));
        event_target->putEvent(new_event);
    }
}


void Variable::setValue(Datum value, MWTime time) {
    setSilentValue(value, time);
    announce(time);
}


void Variable::setValue(const std::vector<Datum> &indexOrKeyPath, Datum value, MWTime time) {
    setSilentValue(indexOrKeyPath, value, time);
    announce(time);
}


std::string Variable::getVariableName() const {
    return properties.getTagName();
}


// Get and setValue shortcuts
void Variable::operator=(long a) {
    setValue(a);
}
    
void Variable::operator=(int a) {
    setValue((long)a);
}

void Variable::operator=(short a) {
    setValue((long)a);
}

void Variable::operator=(double a) {
   setValue(a);
}
    
void Variable::operator=(float a) {
   setValue((double)a);
}

void Variable::operator=(bool a) {
    setValue(a);
}

void Variable::operator=(MWTime a) {
    setValue((long)a);
}

void Variable::operator=(std::string a){
	setValue(Datum(a));
}

void Variable::operator=(Datum a) {
    setValue(a);
}

Variable::operator MWTime() {
	return (MWTime)getValue(); 
}

    
Variable::operator long() {
	return (long)getValue();
}
    
Variable::operator int() {
	return (int)getValue();
}
    
Variable::operator short() {
	return (short)getValue();
}

Variable::operator double() {
	return (double)getValue();
}

Variable::operator float() {
    return (float)getValue();
}
    
Variable::operator bool() {
	return (bool)getValue();
}

Variable::operator Datum(){
	return getValue();
}


END_NAMESPACE_MW
