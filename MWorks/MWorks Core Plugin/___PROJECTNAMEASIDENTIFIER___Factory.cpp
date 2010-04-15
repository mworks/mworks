/*
 *  ___PROJECTNAME___Factory.cpp
 *  ___PROJECTNAME___
 *
 *  Created by ___FULLUSERNAME___ on ___DATE___.
 *  Copyright ___YEAR___ ___ORGANIZATIONNAME___. All rights reserved.
 *
 */

#include "___PROJECTNAME___Factory.h"

#include <boost/regex.hpp>
#include "MWorksCore/ComponentRegistry.h"

using namespace mw;

shared_ptr<mw::Component> ___PROJECTNAME___Factory::createObject(std::map<std::string, std::string> parameters,
                                                               mw::ComponentRegistry *reg) {
	REQUIRE_ATTRIBUTES(parameters, 
					   "tag",
                       "another_attribute");
	
	std::string tagname(parameters["tag"]);

    
	shared_ptr<Variable> another_attribute = reg->getVariable(parameters["another_attribute"]);	
	
	shared_ptr <___PROJECTNAME___> new_component = shared_ptr<___PROJECTNAME___>(new ___PROJECTNAME___(tagname, another_attribute));
	
	return new_component;
}
