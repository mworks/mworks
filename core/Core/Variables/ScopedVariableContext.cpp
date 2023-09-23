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


void ScopedVariableContext::inheritFrom(const boost::shared_ptr<ScopedVariableContext> &info_to_inherit) {
    for (const auto &item : info_to_inherit->data) {
        auto iter = transparency.emplace(item.first, M_TRANSPARENT).first;  // Does not replace existing value, if any
        if (iter->second == M_TRANSPARENT) {
            data[item.first] = item.second;
        }
    }
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


END_NAMESPACE_MW
