/*
 *  ParameterInfo.h
 *  MWorksCore
 *
 *  Created by Christopher Stawarz on 3/24/11.
 *  Copyright 2011 MIT. All rights reserved.
 *
 */

#ifndef _MW_PARAMETER_INFO_H
#define _MW_PARAMETER_INFO_H

#include <string>

#include "Map.h"


BEGIN_NAMESPACE_MW


class ParameterInfo {
    
public:
    explicit ParameterInfo(bool required = true) :
        required(required)
    { }
    
    explicit ParameterInfo(const std::string &defaultValue) :
        required(true),
        defaultValue(defaultValue)
    { }

    // Without this constructor, ParameterInfo("foo") becomes ParameterInfo(true),
    // because char* can be cast to bool
    explicit ParameterInfo(const char *defaultValue) :
        required(true),
        defaultValue(defaultValue)
    { }
    
    bool isRequired() const {
        return required;
    }
    
    bool hasDefaultValue() const {
        return !(defaultValue.empty());
    }
    
    const std::string& getDefaultValue() const {
        return defaultValue;
    }
    
private:
    bool required;
    std::string defaultValue;
    
};


typedef Map<ParameterInfo> ParameterInfoMap;


END_NAMESPACE_MW


#endif























