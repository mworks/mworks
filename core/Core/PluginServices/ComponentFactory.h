#ifndef OBJECT_FACTORY_H
#define OBJECT_FACTORY_H

/*
 *  ObjectFactory.h
 *  Experimental Control with Cocoa UI
 *
 *  Created by David Cox on Sat Dec 28 2002.
 *  Copyright (c) 2002 MIT. All rights reserved.
 * Paul Jankunas on 1/24/06 - Adding virtual destructor.
 */

#include "Component.h"
#include "ComponentFactoryException.h"
#include "ComponentInfo.h"
#include <boost/shared_ptr.hpp>
#include <map>
#include <string>
#include <vector>


BEGIN_NAMESPACE_MW


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

#define CHECK_ATTRIBUTE(variable, parameters, key) \
    checkAttribute((variable), (parameters)["reference_id"], (key), parameters[(key)])
    

class ComponentRegistry;  // forward declaration
class ParameterValue;
class Variable;


class ComponentFactory {
    
public:
    using StdStringMap = std::map<std::string, std::string>;
    using StdStringVector = std::vector<std::string>;
    
    ComponentFactory() { }
    explicit ComponentFactory(const ComponentInfo &info) : info(info) { }
    
    virtual ~ComponentFactory() { }
    
    const ComponentInfo& getComponentInfo() const {
        return info;
    }
    
    virtual ComponentPtr createObject(StdStringMap parameters, ComponentRegistry *reg) {
        return ComponentPtr();
    }
    
protected:
    bool shouldIgnoreParameter(const std::string &name) const;
    void processParameters(const StdStringMap &parameters, ComponentRegistry *reg, Map<ParameterValue> &values) const;
    
    void requireAttributes(StdStringMap &parameters, const StdStringVector &attributes) const;
    void checkAttribute(ComponentPtr component,
                        const std::string &refID,
                        const std::string &name,
                        const std::string &value) const;
    
private:
    const ComponentInfo info;
    
};


END_NAMESPACE_MW


#endif
