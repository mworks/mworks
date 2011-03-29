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


BEGIN_NAMESPACE_MW


class ComponentInfo {
    
public:
    typedef std::vector<std::string> StdStringVector;
    
    void setSignature(const std::string &newSignature) {
        signature = newSignature;
    }
    
    void addParameter(const std::string &name, bool required = true) {
        addParameter(name, ParameterInfo(required));
    }
    
    void addParameter(const std::string &name, const std::string &defaultValue) {
        addParameter(name, ParameterInfo(defaultValue));
    }
    
    // Without this overload, addParameter("foo", "bar") becomes addParameter("foo", true),
    // because char* can be cast to bool
    void addParameter(const std::string &name, const char *defaultValue) {
        addParameter(name, ParameterInfo(defaultValue));
    }
    
    void addParameter(const std::string &name, const ParameterInfo &info) {
        parameters[name] = info;
        if (info.isRequired()) {
            requiredParameters.push_back(name);
        }
    }
    
    ParameterInfo& getParameter(const std::string &name) {
        return parameters.at(name);
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


END_NAMESPACE_MW


#endif























