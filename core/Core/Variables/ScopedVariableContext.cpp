/*
*  ScopedVariableContext.cpp
*  Experimental Control with Cocoa UI
*
*  Created by David Cox on Wed Dec 11 2002.
*  Copyright (c) 2002 __MyCompanyName__. All rights reserved.
*
*/

#include "ScopedVariableContext.h"


BEGIN_NAMESPACE_MW


ScopedVariableContext::ScopedVariableContext(const boost::shared_ptr<ScopedVariableEnvironment> &environment) {
    const auto nfields = environment->getNVariables();
    for (int i = 0; i < nfields; i++) {
        data.emplace(i, Datum());
        transparency.emplace(i, M_TRANSPARENT);
    }
}


void ScopedVariableContext::inheritFrom(const boost::shared_ptr<ScopedVariableContext> &info_to_inherit) {
    const int nfields = data.size();
    for (int i = 0; i < nfields; i++) {
        if (M_TRANSPARENT == transparency[i]) {
            data[i] = info_to_inherit->get(i);
        }
    }
}


Transparency ScopedVariableContext::getTransparency(const boost::shared_ptr<ScopedVariable> &var) const {
    return getTransparency(var->getContextIndex());
}


Transparency ScopedVariableContext::getTransparency(int i) const {
    auto iter = transparency.find(i);
    if (iter == transparency.end()) {
        merror(M_SYSTEM_MESSAGE_DOMAIN, "Attempt to access invalid index in variable context");
        return M_TRANSPARENT;
    }
    return iter->second;
}


Datum ScopedVariableContext::get(int index) const {
    auto iter = data.find(index);
    if (iter == data.end()) {
        merror(M_SYSTEM_MESSAGE_DOMAIN, "Attempt to access invalid index in variable context");
        return Datum(0L);
    }
    return iter->second;
}


void ScopedVariableContext::set(int index, const Datum &newdata) {
    data[index] = newdata;
    transparency[index] = M_OPAQUE;
}


void ScopedVariableContext::setWithTransparency(int index, const Datum &newdata) {
    data[index] = newdata;
    transparency[index] = M_TRANSPARENT;
}


END_NAMESPACE_MW
