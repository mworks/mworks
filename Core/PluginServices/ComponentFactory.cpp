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
            // Parameters added by the parser
            //
            (name == "reference_id") ||
            (name == "type") ||
            (name == "variable_assignment") ||
            (name == "working_path") ||
            (name == "xml_document_path") ||

            //
            // Generic, currently-unused parameters that appear in many old experiments
            //
            (name == "full_name")
            );
}


void ComponentFactory::processParameters(StdStringMap &parameters, ComponentRegistryPtr reg, ParameterValueMap &values)
{
    requireAttributes(parameters, info.getRequiredParameters(), true);
    
    const ParameterInfoMap &infoMap = info.getParameters();
    
    for (StdStringMap::const_iterator param = parameters.begin(); param != parameters.end(); param++) {
        const std::string &name = (*param).first;
        ParameterInfoMap::const_iterator iter = infoMap.find(name);
        
        if ((iter == infoMap.end()) && !shouldIgnoreParameter(name)) {
            throw UnknownAttributeException(name);
        }
        
        const std::string &value = (*param).second;
        const ParameterInfo &info = (*iter).second;
        
        values.insert(std::make_pair(name, ParameterValue(value, reg)));
    }
}


void ComponentFactory::requireAttributes(StdStringMap &parameters,
                                         const StdStringVector &requiredAttributes,
                                         bool useDefaults)
{
    for (StdStringVector::const_iterator attr = requiredAttributes.begin(); attr != requiredAttributes.end(); attr++)
    {
        if (parameters.find(*attr) == parameters.end()) {
            if (useDefaults) {
                const ParameterInfo &param = info.getParameters()[*attr];
                if (param.hasDefaultValue()) {
                    parameters[*attr] = param.getDefaultValue();
                    continue;
                }
            }
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























