/*
 *  ScopedVariable.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 3/25/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */


#include "ScopedVariable.h"
#include "ExpressionVariable.h"
#include "Experiment.h"
#include "ScopedVariableEnvironment.h"


BEGIN_NAMESPACE_MW


/*******************************************************************
*                   ScopedVariable member functions
*******************************************************************/
ScopedVariable::ScopedVariable(const VariableProperties &props) : Variable(props) {
    context_index = -1;
	environment = NULL;
}


Datum ScopedVariable::getValue(){  

	if(environment == NULL){
		mwarning(M_SYSTEM_MESSAGE_DOMAIN,
				 "Scoped variable belongs to invalid (NULL) environmnet");
		return Datum(0L); // don't crash
	}

	return environment->getValue(context_index); 
}


void ScopedVariable::setSilentValue(Datum data, MWTime timeUS) {
	if(environment == NULL){
		mwarning(M_SYSTEM_MESSAGE_DOMAIN,
				 "Scoped variable belongs to invalid (NULL) environmnet");
		return; // don't crash
	}
	
	environment->setValue(context_index, data);
    performNotifications(std::move(data), timeUS);
}


void ScopedVariable::setSilentValue(const std::vector<Datum> &indexOrKeyPath, Datum value, MWTime timeUS) {
    if (!environment) {
        mwarning(M_SYSTEM_MESSAGE_DOMAIN,
                 "Scoped variable belongs to invalid (NULL) environmnet");
        return;
    }
    
    Datum data = environment->getValue(context_index);
    data.setElement(indexOrKeyPath, value);
    environment->setValue(context_index, data);
    performNotifications(std::move(data), timeUS);
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


END_NAMESPACE_MW
