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

	ConstantVariable(Data _value) : GlobalVariable(_value, NULL){ }

	// TODO: warn?
	virtual void setValue(Data _value){ *value = _value;}
	virtual void setValue(Data _value, MonkeyWorksTime time){  *value = _value;}
	virtual void setSilentValue(Data _value){ return; }


};
}
#endif

