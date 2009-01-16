/*
 *  ObjectFactory.cpp
 *  Experimental Control with Cocoa UI
 *
 *  Created by David Cox on Sun Dec 29 2002.
 *  Copyright (c) 2002 __MyCompanyName__. All rights reserved.
 *
 */

#include "ComponentFactory.h"
using namespace mw;

namespace mw {
	const mw::Component *InvalidObject = new mw::Component();
}

void ComponentFactory::requireAttributes(const std::map<std::string, std::string> &parameters,
										  const std::vector<std::string> &requiredAttributes) {
	
	for(std::vector<std::string>::const_iterator i = requiredAttributes.begin();
		i != requiredAttributes.end();
		++i) {
		std::map<std::string, std::string>::const_iterator attribute = parameters.find(*i);
		if(attribute == parameters.end()) {
			string reference_id("<unknown object>");
            if(parameters.find("reference_id") != parameters.end()){
                reference_id = parameters.find("reference_id")->second;
            }
            
			throw MissingAttributeException(reference_id, 
											 "object is missing attribute " + *i);
		}
	}
}

void ComponentFactory::checkAttribute(const shared_ptr<mw::Component> &component,
									   const std::string &refID,
									   const std::string &name,
									   const std::string &value) {
	if(component == 0) {
		throw MissingReferenceException(refID, name, value);		
	} else if(component.get() == InvalidObject) {
		throw InvalidReferenceException(refID, name, value);				
	}
}



