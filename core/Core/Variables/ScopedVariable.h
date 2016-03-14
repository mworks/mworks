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


BEGIN_NAMESPACE_MW


class ScopedVariableEnvironment;

class ScopedVariable : public Variable {
	
	protected:
	
	int context_index;
	ScopedVariableEnvironment *environment; // delegate for get and set methods
	

	public:
	
	ScopedVariable(VariableProperties *_properties);


	// Accessors
	ScopedVariableEnvironment *getEnvironment();
	void setEnvironment(ScopedVariableEnvironment *_env);

	int getContextIndex();
	void setContextIndex(int i);


	// The scoped variable delegates its get and set methods
    Datum getValue() override;
    void setSilentValue(Datum _value, MWTime _when) override;
    void setSilentValue(const std::vector<Datum> &indexOrKeyPath, Datum value, MWTime when) override;
    
    bool isWritable() const override { return true; }
	
};


END_NAMESPACE_MW


#endif
