/*
 *  ___PROJECTNAMEASIDENTIFIER___Factory.cpp
 *  ___PROJECTNAME___
 *
 *  Created by ___FULLUSERNAME___ on ___DATE___.
 *  Copyright ___YEAR___ ___ORGANIZATIONNAME___. All rights reserved.
 *
 */

#include <MWorksCore/ComponentRegistry.h>

#include "___PROJECTNAMEASIDENTIFIER___Factory.h"
#include "___PROJECTNAMEASIDENTIFIER___.h"

using namespace mw;


shared_ptr<mw::Component> ___PROJECTNAMEASIDENTIFIER___Factory::createObject(std::map<std::string, std::string> parameters,
                                                                             ComponentRegistry *reg)
{
    const char* TAG = "tag";
    const char* ANOTHER_ATTRIBUTE = "another_attribute";

    REQUIRE_ATTRIBUTES(parameters,
                       TAG,
                       ANOTHER_ATTRIBUTE);
    
    std::string tag(parameters[TAG]);
    
    shared_ptr<Variable> anotherAttribute(reg->getVariable(parameters[ANOTHER_ATTRIBUTE]));
    CHECK_ATTRIBUTE(anotherAttribute, parameters, ANOTHER_ATTRIBUTE);
    
    shared_ptr<___PROJECTNAMEASIDENTIFIER___> newComponent(new ___PROJECTNAMEASIDENTIFIER___(tag,
                                                                                             anotherAttribute));
    
    return newComponent;
}
