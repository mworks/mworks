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
#include "ConstantVariable.h"
#include "ExpressionVariable.h"
#include "GlobalVariable.h"
#include "ScopedVariable.h"
#include "Experiment.h"
#include "Event.h"
#include "EventBuffer.h"
#include <boost/algorithm/string/case_conv.hpp>
#include "ComponentRegistry_new.h"
#include "EventBuffer.h"
#include "TrialBuildingBlocks.h"

using namespace mw;


/*******************************************************************
*                   Variable member functions
*******************************************************************/
Variable::Variable(VariableProperties *_properties) : mw::Component() {
    codec_code = -1;
    properties = _properties;
	
	if(properties != NULL){
		logging = properties->getLogging();
	}
	
	
	buffer_manager = GlobalBufferManager;
	
}

Variable::Variable(const Variable& that) {
   
    codec_code = that.codec_code;
    logging = that.logging;
	if(that.properties != NULL){
		properties = new VariableProperties(*(that.properties));
	} else {
		properties = NULL;
	}
	
	notifications = that.notifications;
}


Variable *Variable::frozenClone(){
	ConstantVariable *frozen = new ConstantVariable(getValue());
	return (Variable *)frozen;
}

// Factory method
shared_ptr<mw::Component> VariableFactory::createObject(std::map<std::string, std::string> parameters,
											   mwComponentRegistry *reg){
	REQUIRE_ATTRIBUTES(parameters,
					   "tag",
					   "type",
					   "default_value",
					   "scope");
	std::string tag;
	std::string full_name("");
	std::string description("");
	std::string type_string("");
	std::string editable_string("");
	std::string viewable_string("");
	std::string persistant_string("");
	std::string logging_string("");
	std::string scope_string("");
	
	GenericDataType type = M_INTEGER;
	WhenType editable = M_ALWAYS; // when can we edit the variable
	bool viewable = true; // can the user see this variable
	bool persistant = false; // save the variable from run to run
	WhenType logging = M_WHEN_CHANGED; // when does this variable get logged
	Data defaultValue(0L); // the default value Data object.	
	std::string groups("");
	
	GET_ATTRIBUTE(parameters, tag, "tag", "NO_TAG");
	GET_ATTRIBUTE(parameters, full_name, "full_name", "");	
	GET_ATTRIBUTE(parameters, description, "description", "");	
	GET_ATTRIBUTE(parameters, type_string, "type", "double");
	GET_ATTRIBUTE(parameters, editable_string, "editable", "always");
	GET_ATTRIBUTE(parameters, viewable_string, "viewable", "1");
	GET_ATTRIBUTE(parameters, persistant_string, "persistant", "0");
	GET_ATTRIBUTE(parameters, logging_string, "logging", "never");
	GET_ATTRIBUTE(parameters, scope_string, "scope", "global");


	type_string = to_lower_copy(type_string);
	
	if(type_string == "integer") {
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
	} else {
		throw InvalidAttributeException(parameters.find("reference_id")->second, "type", parameters.find("type")->second);
	}
	
	
	// TODO...when changed?
	editable_string = to_lower_copy(editable_string);
	
	if(editable_string == "never") {
		editable = M_NEVER;
	} else if(editable_string == "when_idle") {
		editable = M_WHEN_IDLE;
	} else if(editable_string == "always") {
		editable = M_ALWAYS;
	} else if(editable_string == "at_startup") {
		editable = M_AT_STARTUP;
	} else if(editable_string == "every_trial") {
		editable = M_EVERY_TRIAL;
	} else if(editable_string == "when_changed") {
		editable = M_WHEN_CHANGED;
	} else {
		throw InvalidAttributeException(parameters.find("reference_id")->second, "editable", parameters.find("editable")->second);
	}


	try {
		viewable = reg->getBoolean(viewable_string);
	} catch (boost::bad_lexical_cast &) {
		throw InvalidAttributeException(parameters.find("reference_id")->second, "viewable", parameters.find("viewable")->second);
	}
	
	try {
		persistant = reg->getBoolean(persistant_string);
	} catch (boost::bad_lexical_cast &) {
		throw InvalidAttributeException(parameters.find("reference_id")->second, "persistant", parameters.find("persistant")->second);
	}
	
	
	if(to_lower_copy(logging_string) == "never") {
		logging = M_NEVER;
	} else if(to_lower_copy(logging_string) == "when_idle") {
		logging = M_WHEN_IDLE;
	} else if(to_lower_copy(logging_string) == "always" ){
		logging = M_ALWAYS;
	} else if(to_lower_copy(logging_string) == "at_startup") {
		logging = M_AT_STARTUP;
	} else if(to_lower_copy(logging_string) == "every_trial") {
		logging = M_EVERY_TRIAL;
	} else if(to_lower_copy(logging_string) == "when_changed") {
		logging = M_WHEN_CHANGED;
	} else {
		throw InvalidAttributeException(parameters.find("reference_id")->second, "logging", logging_string);
	}
	
	
	
	switch(type) {
		case M_INTEGER:
		case M_FLOAT:
		case M_BOOLEAN:
			try {
				defaultValue = reg->getNumber(parameters.find("default_value")->second);
				//mData(type, boost::lexical_cast<double>(parameters.find("default_value")->second));					
			} catch (bad_lexical_cast &) {					
				throw InvalidAttributeException(parameters.find("reference_id")->second, "default_value", parameters.find("default_value")->second);
			}
			break;
		case M_STRING:
			defaultValue = Data(parameters.find("default_value")->second);
			break;
		default:
			throw InvalidAttributeException(parameters.find("reference_id")->second, "default_value", parameters.find("default_value")->second);
			break;
	}
	
	if(parameters.find("groups") != parameters.end()) {
		groups = parameters.find("groups")->second;
	}
	
	// TODO when the variable properties get fixed, we can get rid of this nonsense
	VariableProperties props(&defaultValue,
							  tag,
							  full_name,
							  description,
							  editable,
							  logging,
							  viewable,
							  persistant,
							  M_INTEGER_INFINITE,
							  groups);
	
	shared_ptr<mw::Component>newVar;
	
	scope_string = to_lower_copy(scope_string);
	if(scope_string == "global") {
		newVar = GlobalVariableRegistry->createGlobalVariable(&props);
	} else if(scope_string == "local") {
		newVar = GlobalVariableRegistry->createScopedVariable(GlobalCurrentExperiment, &props);		
	} else {
		throw InvalidAttributeException(parameters.find("reference_id")->second, "scope", parameters.find("scope")->second);
	}
	
	return newVar;
}


Variable::~Variable() {
	if(properties) {
		delete properties;
	}
}

void Variable::addChild(std::map<std::string, std::string> parameters,
						 mwComponentRegistry *reg,
						 shared_ptr<mw::Component> child) {
	
	shared_ptr<Action> act = dynamic_pointer_cast<Action,mw::Component>(child);
	
	if(act == 0) {
		throw SimpleException("Attempting to add illegal action (" + child->getTag() + ") to variable (" + this->getVariableName() + ")");
	}
	
	
	shared_ptr<ActionVariableNotification> avn(new ActionVariableNotification(act));
	addNotification(avn);	
}

void Variable::addNotification(shared_ptr<VariableNotification> _notif) {
	if(_notif == shared_ptr<VariableNotification>()){
		mwarning(M_PARADIGM_MESSAGE_DOMAIN,
					"Attempt to add a null notification to a variable");
		return;
	}
	notifications.addToBack(_notif);
}

void Variable::performNotifications(Data data) {
	shared_ptr <Clock> clock = Clock::instance(false);
	if(clock != 0) {
		performNotifications(data, clock->getCurrentTimeUS());
	} else {
		performNotifications(data, 0);
	}		
}

void Variable::performNotifications(Data data, MonkeyWorksTime timeUS) {
	notifications.lock();
	
	shared_ptr<VariableNotification> node = getFirstNotification(); 


	while(node != NULL){
		shared_ptr<VariableNotification> notif = node;
		notif->notify(data, timeUS);
		node = node->getNext();
	}
	
	notifications.unlock();
}



void Variable::announce(){
	shared_ptr <Clock> clock = Clock::instance(false);
	if(clock != 0) {
		announce(clock->getCurrentTimeUS());
	} else {
		announce(0);
	}
}


void Variable::announce(MonkeyWorksTime timeUS){

	if(properties){
		if(properties->getLogging() == M_WHEN_CHANGED && buffer_manager != 0) {
			shared_ptr<Event> new_event(new Event(codec_code, timeUS, getValue()));
			buffer_manager->putEvent(new_event);
		}
	}
}


std::string Variable::getVariableName(){
	std::string returnval = "";
	
	if(properties != NULL){
		returnval = properties->getTagName();
	}
	
	return returnval;
}


WhenType Variable::getLogging() {
    return logging;
}

void Variable::setLogging(WhenType when) {
    logging = when;
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

void Variable::operator=(MonkeyWorksTime a) {
    setValue((long)a);
}

void Variable::operator=(std::string a){
	setValue(Data(a));
}

void Variable::operator=(Data a) {
    setValue(a);
}

Variable::operator MonkeyWorksTime() {
	return (MonkeyWorksTime)getValue(); 
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

Variable::operator Data(){
	return getValue();
}



ExpressionVariable Variable::operator+(Variable& v)
{
	return ExpressionVariable((Variable *)clone(), (Variable *)(v.clone()), M_PLUS);
}

ExpressionVariable Variable::operator-(Variable& v)
{
	return ExpressionVariable((Variable *)clone(), (Variable *)v.clone(), M_MINUS);
}

ExpressionVariable Variable::operator*(Variable& v)
{
	return ExpressionVariable((Variable *)clone(), (Variable *)v.clone(), M_TIMES);
}

ExpressionVariable Variable::operator/(Variable& v)
{
	return ExpressionVariable((Variable *)clone(), (Variable *)v.clone(), M_DIVIDE);
}

ExpressionVariable Variable::operator==(Variable& v)
{
	return ExpressionVariable((Variable *)clone(), (Variable *)v.clone(), M_IS_EQUAL);
}

ExpressionVariable Variable::operator!=(Variable& v)
{
	return ExpressionVariable((Variable *)clone(), (Variable *)v.clone(), M_IS_NOT_EQUAL);
}

ExpressionVariable Variable::operator%(Variable& v)
{
	return ExpressionVariable((Variable *)clone(), (Variable *)v.clone(), M_MOD);
}

ExpressionVariable Variable::operator>(Variable& v)
{
	return ExpressionVariable((Variable *)clone(), (Variable *)v.clone(), M_IS_GREATER_THAN);
}

ExpressionVariable Variable::operator<(Variable& v)
{
	return ExpressionVariable((Variable *)clone(), (Variable *)v.clone(), M_IS_LESS_THAN);
}

ExpressionVariable Variable::operator>=(Variable& v)
{
	return ExpressionVariable((Variable *)clone(), (Variable *)v.clone(), M_IS_GREATER_THAN_OR_EQUAL);
}

ExpressionVariable Variable::operator<=(Variable& v)
{
	return ExpressionVariable((Variable *)clone(), (Variable *)v.clone(), M_IS_LESS_THAN_OR_EQUAL);
}

ExpressionVariable Variable::operator&&(Variable& v)
{
	return ExpressionVariable((Variable *)clone(), (Variable *)v.clone(), M_AND);
}

ExpressionVariable Variable::operator||(Variable& v)
{
	return ExpressionVariable((Variable *)clone(), (Variable *)v.clone(), M_OR);
}

ExpressionVariable Variable::operator!()
{
	return ExpressionVariable((Variable *)clone(), NULL, M_NOT);
}


ExpressionVariable Variable::operator++()
{
	return ExpressionVariable((Variable *)clone(), NULL, M_INCREMENT);
}

ExpressionVariable Variable::operator--()
{
	return ExpressionVariable((Variable *)clone(), NULL, M_DECREMENT);
}

ExpressionVariable Variable::operator-()
{
	return ExpressionVariable((Variable *)clone(), NULL, M_UNARY_MINUS);
}

ExpressionVariable Variable::operator+()
{
	return ExpressionVariable((Variable *)clone(), NULL, M_UNARY_PLUS);
}



