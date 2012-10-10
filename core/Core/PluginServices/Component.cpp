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
    tag(parameters[TAG].str())
{
    compact_id = _id_count++;
}


Component::Component(const std::string &_tag, const std::string &_sig) :
    tag(_tag),
    object_signature(_sig)
{
    compact_id = _id_count++;
}


std::string AmbiguousComponentReference::getStringRepresentation() const {
    std::stringstream rep;
    vector< shared_ptr<mw::Component> >::const_iterator i;
    for(i = ambiguous_components.begin(); i != ambiguous_components.end(); ++i){
        shared_ptr<Component> comp = *i;
        rep << comp->getTag() << "(" << comp->getReferenceID() << ") ";
    }
    
    return rep.str();
}


END_NAMESPACE_MW






















