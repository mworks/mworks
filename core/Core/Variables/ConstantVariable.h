/*
 *  ConstantVariable.h
 *  MWorksCore
 *
 *  Created by David Cox on 3/28/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef	CONSTANT_VARIABLE_H_
#define CONSTANT_VARIABLE_H_

#include "GlobalVariable.h"


BEGIN_NAMESPACE_MW


class ConstantVariable : public GlobalVariable {


public:

	ConstantVariable(Datum _value) : GlobalVariable(_value, NULL){ }

	// TODO: warn?
	virtual void setValue(Datum _value){ *value = _value;}
	virtual void setValue(Datum _value, MWTime time){  *value = _value;}
	virtual void setSilentValue(Datum _value){ return; }
    
    bool isWritable() const MW_OVERRIDE { return false; }


};


END_NAMESPACE_MW


#endif

