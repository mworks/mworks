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


bool ComponentFactory::shouldIgnoreParameter(const std::string &name) {
    return (
            //
            // Parameters added or used by the parser
            //
            (name == "parent_scope") ||
            (name == "reference_id") ||
            (name == "type") ||
            (name == "variable_assignment") ||
            (name == "working_path") ||
            (name == "xml_document_path") ||
            (name == "_line_number") ||

            //
            // Generic, currently-unused parameters that appear in many old experiments
            //
            (name == "full_name") ||
            
            //
            // The editor used to add this, so it's probably still in many experiments
            //
            (name == "_id")
            );
}


void ComponentFactory::processParameters(StdStringMap &parameters, ComponentRegistryPtr reg, ParameterValueMap &values)
{
    const ParameterInfoMap &infoMap = info.getParameters();
    
    for (ParameterInfoMap::const_iterator iter = infoMap.begin(); iter != infoMap.end(); iter++) {
        const std::string &name = (*iter).first;

        if (parameters.find(name) == parameters.end()) {
            const ParameterInfo &info = (*iter).second;

            if (info.isRequired()) {
                if (info.hasDefaultValue()) {
                    parameters[name] = info.getDefaultValue();
                } else {
                    throw MissingAttributeException(name);
                }
            } else {
                parameters[name] = "";
            }
        }
    }
    
    for (StdStringMap::const_iterator param = parameters.begin(); param != parameters.end(); param++) {
        const std::string &name = (*param).first;
        
        if ((infoMap.find(name) == infoMap.end()) && !shouldIgnoreParameter(name)) {
            throw UnknownAttributeException(name);
        }
        
        values.insert(std::make_pair(name, ParameterValue((*param).second, reg)));
    }
}


void ComponentFactory::requireAttributes(StdStringMap &parameters, const StdStringVector &requiredAttributes)
{
    for (StdStringVector::const_iterator attr = requiredAttributes.begin(); attr != requiredAttributes.end(); attr++)
    {
        if (parameters.find(*attr) == parameters.end()) {
            throw MissingAttributeException(*attr);
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























