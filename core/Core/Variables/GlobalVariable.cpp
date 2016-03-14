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
    performNotifications(std::move(newval), timeUS);
}


void GlobalVariable::setSilentValue(const std::vector<Datum> &indexOrKeyPath, Datum elementValue, MWTime timeUS) {
    Datum newval;
    {
        lock_guard lock(valueMutex);
        value.setElement(indexOrKeyPath, elementValue);
        newval = value;
    }
    performNotifications(std::move(newval), timeUS);
}


END_NAMESPACE_MW


























