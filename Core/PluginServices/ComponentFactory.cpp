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


void ComponentFactory::processParameters(const StdStringMap &parameters,
                                         ComponentRegistryPtr reg,
                                         ParameterValueMap &values)
{
    requireAttributes(parameters, info.getRequiredParameters());
    
    const ParameterInfoMap &infoMap = info.getParameters();
    
    for (StdStringMap::const_iterator param = parameters.begin(); param != parameters.end(); param++) {
        const std::string &name = (*param).first;
        ParameterInfoMap::const_iterator iter = infoMap.find(name);
        
        if ((iter == infoMap.end()) && !isInternalParameter(name)) {
            throw UnknownAttributeException(name);
        }
        
        const std::string &value = (*param).second;
        const ParameterInfo &info = (*iter).second;
        
        values.insert(std::make_pair(name, ParameterValue(value, reg)));
    }
}


void ComponentFactory::requireAttributes(const StdStringMap &parameters, const StdStringVector &requiredAttributes)
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























