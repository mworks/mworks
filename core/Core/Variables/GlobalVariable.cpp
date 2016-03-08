/*
 *  GlobalVariable.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 2/20/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include "GlobalVariable.h"
#include "VariableProperties.h"
#include <boost/shared_ptr.hpp>


BEGIN_NAMESPACE_MW


GlobalVariable::GlobalVariable(Datum value, VariableProperties *props) :
    Variable(props),
    value(value)
{ }


GlobalVariable::GlobalVariable(VariableProperties *props) :
    Variable(props)
{
    if (props) {
        value = props->getDefaultValue();
    }
}


Datum GlobalVariable::getValue() {
    lock_guard lock(valueMutex);
    return value;
}


void GlobalVariable::setSilentValue(Datum newval, MWTime timeUS) {
    {
        lock_guard lock(valueMutex);
        value = newval;
    }
	performNotifications(newval, timeUS);
}


END_NAMESPACE_MW


























