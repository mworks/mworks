/*
 *  ParameterValue.h
 *  MWorksCore
 *
 *  Created by Christopher Stawarz on 3/24/11.
 *  Copyright 2011 MIT. All rights reserved.
 *
 */


#ifndef _MW_PARAMETER_VALUE_H
#define _MW_PARAMETER_VALUE_H

#include <map>
#include <string>

#include <boost/lexical_cast.hpp>

#include "ComponentRegistry.h"
#include "Map.h"
#include "ParsedColorTrio.h"
#include "Stimulus.h"


BEGIN_NAMESPACE_MW


class ParameterValue {
    
public:
    ParameterValue(const std::string &value, ComponentRegistryPtr reg) :
        value(value),
        reg(reg)
    { }
    
    ComponentRegistryPtr getRegistry() const {
        return reg;
    }
    
    bool empty() const {
        return value.empty();
    }
    
    const std::string& str() const {
        return value;
    }
    
    template<typename Type>
    operator Type() const {
        return as<Type>();
    }
    
    template<typename Type>
    Type as() const {
        return convert<Type>(value, reg);
    }
    
    template<typename Type>
    static Type convert(const std::string &s, ComponentRegistryPtr reg);
    
private:
    std::string value;
    ComponentRegistryPtr reg;
    
};


typedef Map<ParameterValue> ParameterValueMap;


template<typename Type>
Type ParameterValue::convert(const std::string &s, ComponentRegistryPtr reg) {
    Type val;
    
    try {
        val = boost::lexical_cast<Type>(s);
    } catch (boost::bad_lexical_cast &e) {
        val = Type(convert<VariablePtr>(s, reg)->getValue());
    }
    
    return val;
}


template<>
VariablePtr ParameterValue::convert(const std::string &s, ComponentRegistryPtr reg);


template<>
ParsedColorTrio ParameterValue::convert(const std::string &s, ComponentRegistryPtr reg);


template<>
RGBColor ParameterValue::convert(const std::string &s, ComponentRegistryPtr reg);


template<>
StimulusNodePtr ParameterValue::convert(const std::string &s, ComponentRegistryPtr reg);


template<>
StimulusGroupPtr ParameterValue::convert(const std::string &s, ComponentRegistryPtr reg);


END_NAMESPACE_MW


#endif























