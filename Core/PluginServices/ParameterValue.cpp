/*
 *  ParameterValue.cpp
 *  MWorksCore
 *
 *  Created by Christopher Stawarz on 3/24/11.
 *  Copyright 2011 MIT. All rights reserved.
 *
 */

#include "ParameterValue.h"


BEGIN_NAMESPACE_MW


template<>
VariablePtr ParameterValue::convert(const std::string &s, ComponentRegistryPtr reg) {
    return reg->getVariable(s);
}


template<>
ParsedColorTrio ParameterValue::convert(const std::string &s, ComponentRegistryPtr reg) {
    return ParsedColorTrio(reg, s);
}


template<>
RGBColor ParameterValue::convert(const std::string &s, mw::ComponentRegistry *reg) {
    return convert<ParsedColorTrio>(s, reg).getValue();
}


template<>
StimulusGroupPtr ParameterValue::convert(const std::string &s, ComponentRegistryPtr reg) {
    StimulusGroupPtr stimGroup(reg->getObject<StimulusGroup>(s));

    if (!stimGroup) {
        throw SimpleException("Unknown stimulus group", s);
    }

    return stimGroup;
}


END_NAMESPACE_MW






















