/*
 *  ScopedVariableEnvironment.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 3/26/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "ScopedVariableEnvironment.h"

#include "ScopedVariableContext.h"


BEGIN_NAMESPACE_MW


int ScopedVariableEnvironment::addVariable(const boost::shared_ptr<ScopedVariable> &var) {
    auto index = variables.size();
    var->setContextIndex(index);
    variables.emplace_back(var);
    return index;
}


boost::shared_ptr<ScopedVariableContext> ScopedVariableEnvironment::createNewContext() {
    return boost::make_shared<ScopedVariableContext>(this);
}


boost::shared_ptr<ScopedVariableContext> ScopedVariableEnvironment::createNewDefaultContext() {
    auto new_context = createNewContext();
    for (auto &var : variables) {
        new_context->set(var->getContextIndex(), var->getProperties().getDefaultValue());
    }
    return new_context;
}


Datum ScopedVariableEnvironment::getValue(int index) {
    if (current_context) {
        return current_context->get(index);
    }
    merror(M_PARADIGM_MESSAGE_DOMAIN, "Scoped variable environment has no current context");
    return Datum();
}


void ScopedVariableEnvironment::setValue(int index, Datum value) {
    if (current_context) {
        current_context->set(index, value);
    } else {
        merror(M_PARADIGM_MESSAGE_DOMAIN, "Scoped variable environment has no current context");
    }
}


void ScopedVariableEnvironment::announceAll() {
    for (auto &var : variables) {
        var->announce();
    }
}


END_NAMESPACE_MW
