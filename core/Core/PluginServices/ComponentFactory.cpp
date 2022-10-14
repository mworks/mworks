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


bool ComponentFactory::shouldIgnoreParameter(const std::string &name) const {
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
            (name == "_line_number") ||  // No longer used, but may be in experiments extracted from event files
            (name == "_location") ||
            
            //
            // Generic, currently-unused parameters that appear in many old experiments
            //
            (name == "full_name") ||
            
            //
            // The editor used to add this, so it's probably still in many experiments
            //
            (name == "_id") ||
            
            //
            // Other parameters ignored by the component type
            //
            info.shouldIgnoreParameter(name)
            );
}


void ComponentFactory::processParameters(const StdStringMap &parameters,
                                         ComponentRegistryPtr reg,
                                         ParameterValueMap &values) const
{
    //
    // Process the provided parameters
    //
    for (auto &param : parameters) {
        auto name = param.first;  // Copy for passing to resolveParameterAlias
        
        if (!(info.hasParameter(name) || info.resolveParameterAlias(name))) {
            if (!shouldIgnoreParameter(name)) {
                throw UnknownAttributeException(name);
            }
        } else {
            if (!(values.emplace(name, ParameterValue(param.second, reg)).second)) {
                throw DuplicateAttributeException(name);
            }
        }
    }
    
    //
    // Handle any missing parameters
    //
    for (auto &param : info.getParameters()) {
        auto &name = param.first;
        
        if (values.find(name) == values.end()) {
            auto &paramInfo = param.second;
            
            if (paramInfo.hasDefaultValue()) {
                values.emplace(name, ParameterValue(paramInfo.getDefaultValue(), reg));
            } else if (!paramInfo.isRequired()) {
                values.emplace(name, ParameterValue("", reg));
            } else {
                throw MissingAttributeException(name);
            }
        }
    }
}


void ComponentFactory::requireAttributes(StdStringMap &parameters, const StdStringVector &requiredAttributes) const
{
    for (auto &attr : requiredAttributes) {
        if (parameters.find(attr) == parameters.end()) {
            throw MissingAttributeException(attr);
        }
    }
}


void ComponentFactory::checkAttribute(ComponentPtr component,
                                      const std::string &refID,
                                      const std::string &name,
                                      const std::string &value) const
{
	if (!component) {
		throw MissingReferenceException(refID, name, value);		
	}
}


END_NAMESPACE_MW
