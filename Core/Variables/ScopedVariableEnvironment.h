/*
 *  ScopedVariableEnvironment.h
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 3/26/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef	VARIABLE_ENVIRONMENT_H_
#define VARIABLE_ENVIRONMENT_H_

#include "ExpandableList.h"
#include "ScopedVariableContext.h"
namespace mw {
class ScopedVariableContext;

class ScopedVariable;

class ScopedVariableEnvironment {

	protected:
	
		ExpandableList<ScopedVariable> variables;
		shared_ptr<ScopedVariableContext> current_context;

	public:

		ScopedVariableEnvironment();
		
		// Add a new scoped variable to this environment
		int addVariable(shared_ptr<ScopedVariable> var);

	
		// Generate a new context that is valid in this environment
		// Should be called *after* all variables are added to the environment
		shared_ptr<ScopedVariableContext> createNewContext();
		shared_ptr<ScopedVariableContext> createNewDefaultContext();
		
		// Set the current variable context
		// Should have been created by the createNewContext method
		void setCurrentContext(shared_ptr<ScopedVariableContext> new_context);

		shared_ptr<ScopedVariableContext> getCurrentContext(){
			return current_context;
		}
		
		int getNVariables(){
			return variables.getNElements();
		}
	
		// ScopedVariable delegate methods
		Data getValue(int index);
		void setValue(int index, Data value);
		
		
		
		// Announcing
		void announceAll();
};
}
#endif

