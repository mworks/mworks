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
    Variable(props),
    context_index(-1)
{ }


Datum ScopedVariable::getValue() {
    if (auto environment = getEnvironment()) {
        return environment->getValue(context_index);
    }
    merror(M_SYSTEM_MESSAGE_DOMAIN, "Scoped variable belongs to invalid (NULL) environment");
    return Datum(0L);
}


void ScopedVariable::setSilentValue(Datum data, MWTime timeUS) {
    auto environment = getEnvironment();
    if (!environment) {
        merror(M_SYSTEM_MESSAGE_DOMAIN, "Scoped variable belongs to invalid (NULL) environment");
        return;
    }
    environment->setValue(context_index, data);
    performNotifications(std::move(data), timeUS);
}


void ScopedVariable::setSilentValue(const std::vector<Datum> &indexOrKeyPath, Datum value, MWTime timeUS) {
    auto environment = getEnvironment();
    if (!environment) {
        merror(M_SYSTEM_MESSAGE_DOMAIN, "Scoped variable belongs to invalid (NULL) environment");
        return;
    }
    auto data = environment->getValue(context_index);
    data.setElement(indexOrKeyPath, value);
    environment->setValue(context_index, data);
    performNotifications(std::move(data), timeUS);
}


END_NAMESPACE_MW
