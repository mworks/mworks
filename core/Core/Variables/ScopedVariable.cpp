/*
 *  ScopedVariable.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 3/25/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "ScopedVariable.h"

#include "ScopedVariableEnvironment.h"


BEGIN_NAMESPACE_MW


ScopedVariable::ScopedVariable(const VariableProperties &props) :
    ReadWriteVariable(props),
    context_index(-1)
{ }


Datum ScopedVariable::getValue() {
    lock_guard lock(valueMutex);
    
    if (auto environment = getEnvironment()) {
        return environment->getValue(context_index);
    }
    
    merror(M_SYSTEM_MESSAGE_DOMAIN, "Scoped variable belongs to invalid (NULL) environment");
    return Datum(0);
}


void ScopedVariable::setValue(const Datum &value, MWTime when, bool silent) {
    lock_guard lock(valueMutex);
    
    auto environment = getEnvironment();
    if (!environment) {
        merror(M_SYSTEM_MESSAGE_DOMAIN, "Scoped variable belongs to invalid (NULL) environment");
        return;
    }
    
    environment->setValue(context_index, value);
    performNotifications(value, when, silent);
}


void ScopedVariable::setValue(const std::vector<Datum> &indexOrKeyPath,
                              const Datum &elementValue,
                              MWTime when,
                              bool silent)
{
    lock_guard lock(valueMutex);
    
    auto environment = getEnvironment();
    if (!environment) {
        merror(M_SYSTEM_MESSAGE_DOMAIN, "Scoped variable belongs to invalid (NULL) environment");
        return;
    }
    
    auto value = environment->getValue(context_index);
    if (value.setElement(indexOrKeyPath, elementValue)) {
        environment->setValue(context_index, value);
        performNotifications(value, when, silent);
    }
}


END_NAMESPACE_MW
