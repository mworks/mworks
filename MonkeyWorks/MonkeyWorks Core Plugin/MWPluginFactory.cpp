/*
 *  ÇPROJECTNAMEÈFactory.cpp
 *  ÇPROJECTNAMEÈ
 *
 *  Created by ÇFULLUSERNAMEÈ on ÇDATEÈ.
 *  Copyright ÇYEARÈ ÇORGANIZATIONNAMEÈ. All rights reserved.
 *
 */

#include "ÇPROJECTNAMEÈFactory.h"

#include <boost/regex.hpp>
#include "MonkeyWorksCore/ComponentRegistry.h"

using namespace mw;

shared_ptr<mw::Component> ÇPROJECTNAMEÈFactory::createObject(std::map<std::string, std::string> parameters,
                                                               mw::ComponentRegistry *reg) {
	REQUIRE_ATTRIBUTES(parameters, 
					   "tag",
                       "another_attribute");
	
	std::string tagname(parameters.find("tag")->second);

    
	shared_ptr<Variable> another_attribute = reg->getVariable(parameters.find("another_attribute")->second);	
	
	shared_ptr <ÇPROJECTNAMEÈ> new_component = shared_ptr<ÇPROJECTNAMEÈ>(new ÇPROJECTNAMEÈ(tagname, another_attribute));
	
	return new_component;
}
