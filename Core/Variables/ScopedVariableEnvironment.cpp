/*
 *  ScopedVariableEnvironment.cpp
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 3/26/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "ScopedVariableEnvironment.h"
#include "ScopedVariable.h"
#include "VariableProperties.h"
using namespace mw;

ScopedVariableEnvironment::ScopedVariableEnvironment() : variables(){
	//current_context = NULL;
}

// Add a new scoped variable to this environment
int ScopedVariableEnvironment::addVariable(shared_ptr<ScopedVariable> var){
	int index = variables.addReference(var);
	var->setContextIndex(index);
	var->setEnvironment(this);
	
	return index;
}

// Generate a new context that is valid in this environment
// Should be called *after* all variables are added to the environment
shared_ptr<ScopedVariableContext> ScopedVariableEnvironment::createNewContext(){
	shared_ptr<ScopedVariableContext> new_context( 
						new ScopedVariableContext(this));
	
	return new_context;
}

shared_ptr<ScopedVariableContext> ScopedVariableEnvironment::createNewDefaultContext(){

	shared_ptr<ScopedVariableContext> new_context = createNewContext();
	
	for(int i = 0; i < variables.getNElements(); i++){
		shared_ptr<ScopedVariable> var = variables[i];
		VariableProperties *props = var->getProperties();
		new_context->set(var->getContextIndex(), props->getDefaultValue());
	}
	
	return new_context;
}


// Set the current variable context
// Should have been created by the createNewContext method
void ScopedVariableEnvironment::setCurrentContext(shared_ptr<ScopedVariableContext> new_context){
	current_context = new_context;
}

// ScopedVariable delegate methods
Data ScopedVariableEnvironment::getValue(int index){
	if(current_context != NULL){
		return current_context->get(index);
	}
	
	// TODO: warn
	return Data();
}


void ScopedVariableEnvironment::setValue(int index, Data value){
	if(current_context != NULL){
		current_context->set(index, value);
	} else {
		// TODO: warn
		merror(M_PARADIGM_MESSAGE_DOMAIN, "Attempt to set a value on a missing context");
		
	}
}


void ScopedVariableEnvironment::announceAll(){
	for(int i = 0; i < variables.getNElements(); i++){
		shared_ptr<ScopedVariable> var = variables[i];
		if(var != shared_ptr<ScopedVariable>()){
			var->announce();
			//TODO Maybe this needs to be here?
			//var->performNotifications();

		}
	}
}
