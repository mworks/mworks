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

#include "ParameterInfo.h"


BEGIN_NAMESPACE_MW


class ComponentInfo {
    
public:
    void setSignature(const std::string &newSignature) {
        signature = newSignature;
    }
    
    void setDisplayName(const std::string &newDisplayName) {
        displayName = newDisplayName;
    }
    
    void setDescription(const std::string &newDescription) {
        description = newDescription;
    }
    
    void addParameter(const std::string &name, bool required = true, const std::string &exampleValue = "") {
        addParameter(name, ParameterInfo(required, exampleValue));
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
    }
    
    const std::string& getSignature() const {
        return signature;
    }
    
    const std::string& getDisplayName() const {
        return displayName;
    }
    
    const std::string& getDescription() const {
        return description;
    }
    
    const ParameterInfoMap& getParameters() const {
        return parameters;
    }
    
private:
    std::string signature;
    std::string displayName;
    std::string description;
    ParameterInfoMap parameters;
    
};


END_NAMESPACE_MW


#endif























