/*
 *  ComponentInfo.h
 *  MWorksCore
 *
 *  Created by Christopher Stawarz on 3/24/11.
 *  Copyright 2011 MIT. All rights reserved.
 *
 */

#ifndef _MW_COMPONENT_INFO_H
#define _MW_COMPONENT_INFO_H

#include <string>
#include <vector>

#include "ParameterInfo.h"


BEGIN_NAMESPACE(mw)


class ComponentInfo {
    
public:
    typedef std::vector<std::string> StdStringVector;
    
    void setSignature(const std::string &newSignature) {
        signature = newSignature;
    }
    
    void addParameter(const std::string &name, const std::string &defaultValue) {
        addParameter(name, ParameterInfo(defaultValue));
    }
    
    void addParameter(const std::string &name, bool required = true) {
        addParameter(name, ParameterInfo(required));
    }
    
    void addParameter(const std::string &name, const ParameterInfo &info) {
        parameters[name] = info;
        if (info.isRequired()) {
            requiredParameters.push_back(name);
        }
    }
    
    const std::string& getSignature() const {
        return signature;
    }
    
    const ParameterInfoMap& getParameters() const {
        return parameters;
    }
    
    const StdStringVector& getRequiredParameters() const {
        return requiredParameters;
    }
    
private:
    std::string signature;
    ParameterInfoMap parameters;
    StdStringVector requiredParameters;
    
};


END_NAMESPACE(mw)


#endif























