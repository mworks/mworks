/*
 *  mw::Component.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 11/25/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "Component.h"
#include "ComponentInfo.h"
#include "ParameterValue.h"


BEGIN_NAMESPACE_MW


long Component::_id_count = 0;


const std::string Component::TAG("tag");


void Component::describeComponent(ComponentInfo &info) {
    info.addParameter(TAG, false);
}


Component::Component(const ParameterValueMap &parameters) :
    tag(parameters[TAG].str()),
    line_number(-1)
{
    compact_id = _id_count++;
}


Component::Component(const std::string &_tag) :
    tag(_tag),
    line_number(-1)
{
    compact_id = _id_count++;
}


END_NAMESPACE_MW






















