/*
 *  VariableReference.h
 *  MWorksCore
 *
 *  Created by David Cox on 11/4/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include "GenericVariable.h"


BEGIN_NAMESPACE_MW


class VariableReference : public Variable {

protected:

	Variable *variable;

public:

	VariableReference(Variable *var);
	
	virtual Variable *clone(){
		return (Variable *)(new VariableReference(variable));
	}
	
	virtual Datum getValue(){
		return variable->getValue();
	}
	
	virtual void setValue(Datum data){
		variable->setValue(data);
	}
	
	virtual void setValue(Datum data, MWTime time){
		variable->setValue(data, time);
	}

	virtual void setSilentValue(Datum data){
		variable->setSilentValue(data);
	}
    
    bool isWritable() const MW_OVERRIDE {
        return variable->isWritable();
    }
	
};


END_NAMESPACE_MW
