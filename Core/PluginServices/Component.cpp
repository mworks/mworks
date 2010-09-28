/*
 *  mw::Component.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 11/25/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "Component.h"
using namespace mw;

long mw::Component::_id_count = 0;


string AmbiguousComponentReference::getStringRepresentation(){
    stringstream rep;
    vector< shared_ptr<mw::Component> >::iterator i;
    for(i = ambiguous_components.begin(); i != ambiguous_components.end(); ++i){
        shared_ptr<Component> comp = *i;
        rep << comp->getTag() << "(" << comp->getReferenceID() << ") ";
    }
    
    return rep.str();
}