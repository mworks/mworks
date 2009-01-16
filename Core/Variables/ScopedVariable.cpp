/*
 *  ScopedVariable.cpp
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 3/25/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */


#include "ScopedVariable.h"
#include "ConstantVariable.h"
#include "ExpressionVariable.h"
#include "Experiment.h"
#include "ScopedVariableEnvironment.h"

using namespace mw;

/*******************************************************************
*                   ScopedVariable member functions
*******************************************************************/
ScopedVariable::ScopedVariable(VariableProperties *props) : Variable(props){
    context_index = -1;
	environment = NULL;
}

//mScopedVariable::ScopedVariable(ScopedVariableEnvironment *env, 
//								 VariableProperties *props) : Variable(props){
//	context_index = -1;
//	environment = env;
//	if(environment != NULL){
//		environment->addVariable(this); // will expand the environment's context
//										// and set context_index
//	}
//}


ScopedVariable::ScopedVariable(const ScopedVariable& that) :
										Variable((const Variable&)that) {
    context_index = that.context_index;
	environment = that.environment;
}


Data ScopedVariable::getValue(){  

	if(environment == NULL){
		mwarning(M_SYSTEM_MESSAGE_DOMAIN,
				 "Scoped variable belongs to invalid (NULL) environmnet");
		return Data(0L); // don't crash
	}

	return environment->getValue(context_index); 
}

void ScopedVariable::setValue(Data _data){ 
	setSilentValue(_data);
	announce();
}

void ScopedVariable::setValue(Data _data, MonkeyWorksTime _when){
	setSilentValue(_data, _when);
	announce(_when);
}

void ScopedVariable::setSilentValue(Data _data){ 
	if(environment == NULL){
		mwarning(M_SYSTEM_MESSAGE_DOMAIN,
				 "Scoped variable belongs to invalid (NULL) environmnet");
		return; // don't crash
	}
	
	environment->setValue(context_index, _data);
	performNotifications(_data);
}

void ScopedVariable::setSilentValue(Data _data, MonkeyWorksTime timeUS){ 
	if(environment == NULL){
		mwarning(M_SYSTEM_MESSAGE_DOMAIN,
				 "Scoped variable belongs to invalid (NULL) environmnet");
		return; // don't crash
	}
	
	environment->setValue(context_index, _data);
	performNotifications(_data, timeUS);
}




////////////////////////////////////////////////////////////////////////////
// A polymorphic copy constructor (inherited from Clonable)
////////////////////////////////////////////////////////////////////////////
Variable *ScopedVariable::clone(){
	ScopedVariable *returned = 
				new ScopedVariable((const ScopedVariable&)(*this));
	return (Variable *)returned;
}


/////////////////////////////////////////////////
// Accessors
/////////////////////////////////////////////////
ScopedVariableEnvironment *ScopedVariable::getEnvironment() {  
	return environment; 
}

void ScopedVariable::setEnvironment(ScopedVariableEnvironment *_env) { 
	environment = _env; 
}

int ScopedVariable::getContextIndex() { 
	return context_index; 
}

void ScopedVariable::setContextIndex(int i) { 
	context_index = i; 
}

