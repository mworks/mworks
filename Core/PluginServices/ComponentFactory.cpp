/*
 *  ObjectFactory.cpp
 *  Experimental Control with Cocoa UI
 *
 *  Created by David Cox on Sun Dec 29 2002.
 *  Copyright (c) 2002 __MyCompanyName__. All rights reserved.
 *
 */

#include "ComponentFactory.h"
#include "GenericVariable.h"
#include "ComponentRegistry.h"
#include "ParameterValue.h"


BEGIN_NAMESPACE_MW


void ComponentFactory::processParameters(StdStringMap &parameters, ComponentRegistryPtr reg, ParameterValueMap &values)
{
    requireAttributes(parameters, info.getRequiredParameters());
    
    const ParameterInfoMap &infoMap = info.getParameters();
    
    for (StdStringMap::iterator param = parameters.begin(); param != parameters.end(); param++) {
        const std::string &name = (*param).first;
        ParameterInfoMap::const_iterator iter = infoMap.find(name);
        
        if ((iter == infoMap.end()) && !isInternalParameter(name)) {
            std::string referenceID("<unknown object>");
            if (parameters.find("reference_id") != parameters.end()) {
                referenceID = parameters["reference_id"];
            }
            throw UnknownAttributeException(referenceID, name);
        }
        
        const std::string &value = (*param).second;
        const ParameterInfo &info = (*iter).second;
        
        values.insert(std::make_pair(name, ParameterValue(value, reg)));
    }
}


void ComponentFactory::requireAttributes(StdStringMap parameters, StdStringVector requiredAttributes)
{
	for(StdStringVector::const_iterator i = requiredAttributes.begin();
		i != requiredAttributes.end();
		++i) {
		StdStringMap::const_iterator attribute = parameters.find(*i);
		if(attribute == parameters.end()) {
			string reference_id("<unknown object>");
            if(parameters.find("reference_id") != parameters.end()){
                reference_id = parameters["reference_id"];
            }
            
            string tag = parameters["tag"];
            if(tag.empty()){
                tag = "unknown name";
            }
            
			throw MissingAttributeException(reference_id, 
											 "Object <" + tag + "> is missing attribute '" + *i + "'");
		}
	}
}


void ComponentFactory::checkAttribute(shared_ptr<mw::Component> component,
                                      const std::string &refID,
                                      const std::string &name,
                                      const std::string &value)
{
	if(component == 0) {
		throw MissingReferenceException(refID, name, value);		
	}
}


END_NAMESPACE_MW























