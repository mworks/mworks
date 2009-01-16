#ifndef OBJECT_FACTORY_H
#define OBJECT_FACTORY_H

/*
 *  ObjectFactory.h
 *  Experimental Control with Cocoa UI
 *
 *  Created by David Cox on Sat Dec 28 2002.
 *  Copyright (c) 2002 MIT. All rights reserved.
 * Paul Jankunas on 1/24/06 - Adding virtual destructor.
 *  See "ServiceRegistry.h" in the ComponentRegistries section for a discussion
 *  of ObjectFactories and plugins.
 */

#include "Component.h"
#include "ComponentFactoryException.h"
#include <boost/shared_ptr.hpp>
#include <map>
#include <string>
#include <vector>
namespace mw {
extern const mw::Component *InvalidObject;

#define REQUIRE_ATTRIBUTES(parameters, attributes...) {\
													const char *requiredAttributes[] = { attributes }; \
													requireAttributes(parameters, \
																	  std::vector<std::string>(requiredAttributes, \
																							   requiredAttributes + sizeof(requiredAttributes)/sizeof(*requiredAttributes))); \
}


#define	GET_ATTRIBUTE(parameters, variable, key, default_value){\
	if(parameters.find(key) != parameters.end()){	\
		variable = parameters.find(key)->second;	\
	} else {										\
		variable = std::string(default_value);			\
	}												\
}

using namespace boost;

class mwComponentRegistry;  // forward declaration

class ComponentFactory {
protected:
	virtual void requireAttributes(const std::map<std::string, std::string> &parameters,
								   const std::vector<std::string> &attributes);
	virtual void checkAttribute(const shared_ptr<mw::Component> &component,
								const std::string &refID,
								const std::string &name,
								const std::string &value);
	
public:
	ComponentFactory(){ }
	virtual ~ComponentFactory(){ }
	
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												mwComponentRegistry *reg){
		return shared_ptr<mw::Component>();
	}
};
}
#endif

