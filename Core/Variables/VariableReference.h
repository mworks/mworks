/*
 *  VariableReference.h
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 11/4/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include "GenericVariable.h"

namespace mw {
class VariableReference : public Variable {

protected:

	Variable *variable;

public:

	VariableReference(Variable *var);
	
	virtual Variable *clone(){
		return (Variable *)(new VariableReference(variable));
	}
	
	virtual Data getValue(){
		return variable->getValue();
	}
	
	virtual void setValue(Data data){
		variable->setValue(data);
	}
	
	virtual void setValue(Data data, MonkeyWorksTime time){
		variable->setValue(data, time);
	}

	virtual void setSilentValue(Data data){
		variable->setSilentValue(data);
	}
	
};

}
