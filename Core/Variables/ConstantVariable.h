/*
 *  ConstantVariable.h
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 3/28/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef	CONSTANT_VARIABLE_H_
#define CONSTANT_VARIABLE_H_

#include "GlobalVariable.h"

namespace mw {
class ConstantVariable : public GlobalVariable {


public:

	ConstantVariable(Datum _value) : GlobalVariable(_value, NULL){ }

	// TODO: warn?
	virtual void setValue(Datum _value){ *value = _value;}
	virtual void setValue(Datum _value, MonkeyWorksTime time){  *value = _value;}
	virtual void setSilentValue(Datum _value){ return; }


};
}
#endif

