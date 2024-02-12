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


GlobalVariable::GlobalVariable(const Datum &value, const VariableProperties &props) :
    ReadWriteVariable(props),
    value(value)
{ }


GlobalVariable::GlobalVariable(const VariableProperties &props) :
    ReadWriteVariable(props),
    value(props.getDefaultValue())
{ }


Datum GlobalVariable::getValue() {
    shared_lock lock(valueMutex);
    return value;
}


void GlobalVariable::setValue(const Datum &newValue, MWTime when, bool silent) {
    upgrade_lock readLock(valueMutex);
    {
        upgrade_to_unique_lock writeLock(readLock);
        value = newValue;
    }
    performNotifications(value, when, silent);
}


void GlobalVariable::setValue(const std::vector<Datum> &indexOrKeyPath,
                              const Datum &elementValue,
                              MWTime when,
                              bool silent)
{
    upgrade_lock readLock(valueMutex);
    bool success = false;
    {
        upgrade_to_unique_lock writeLock(readLock);
        success = value.setElement(indexOrKeyPath, elementValue);
    }
    if (success) {
        performNotifications(value, when, silent);
    }
}


END_NAMESPACE_MW
