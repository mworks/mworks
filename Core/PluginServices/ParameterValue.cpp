/*
 *  ParameterValue.cpp
 *  MWorksCore
 *
 *  Created by Christopher Stawarz on 3/24/11.
 *  Copyright 2011 MIT. All rights reserved.
 *
 */

#include "ParameterValue.h"


BEGIN_NAMESPACE(mw)


template<>
const std::string& ParameterValue::convert(const std::string &s, ComponentRegistryPtr reg) {
    return s;
}


template<>
VariablePtr ParameterValue::convert(const std::string &s, ComponentRegistryPtr reg) {
    return reg->getVariable(s);
}


END_NAMESPACE(mw)
