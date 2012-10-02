/*
 *  ScopedVariable.h
 *  MWorksCore
 *
 *  Created by David Cox on 3/25/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef	SCOPED_VARIABLE_H_
#define SCOPED_VARIABLE_H_

#include "GenericVariable.h"
namespace mw {
class ScopedVariableEnvironment;

class ScopedVariable : public Variable {
	
	protected:
	
	int context_index;
	ScopedVariableEnvironment *environment; // delegate for get and set methods
	

	public:
	
	ScopedVariable(VariableProperties *_properties);
	
	
	ScopedVariable(const ScopedVariable&);
	
	
	/**
	 *  A polymorphic copy constructor
	 */
	virtual Variable *clone();
	
	
	
//	ScopedVariable(ScarabDatum * scarab_package);
	
	virtual ~ScopedVariable(){ }


	// Accessors
	ScopedVariableEnvironment *getEnvironment();
	void setEnvironment(ScopedVariableEnvironment *_env);

	int getContextIndex();
	void setContextIndex(int i);


	// The scoped variable delegates its get and set methods
	virtual Datum getValue();
	
	virtual void setValue(Datum _data);	
	virtual void setValue(Datum _data, MWTime _when);
	virtual void setSilentValue(Datum _value);
	virtual void setSilentValue(Datum _value,  MWTime _when);
	
};
}

#endif
