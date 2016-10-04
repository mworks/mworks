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

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

#include "ComponentRegistry.h"
#include "Experiment.h"
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
    explicit operator Type() const {
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


using ParameterValueMap = Map<ParameterValue>;


template<typename Type>
Type ParameterValue::convert(const std::string &s, ComponentRegistryPtr reg) {
    try {
        return boost::lexical_cast<Type>(s);
    } catch (const boost::bad_lexical_cast &) {
        return Type(convert<VariablePtr>(s, reg)->getValue());
    }
}


template<>
inline VariablePtr ParameterValue::convert(const std::string &s, ComponentRegistryPtr reg) {
    return reg->getVariable(s);
}


template<>
inline ParsedColorTrio ParameterValue::convert(const std::string &s, ComponentRegistryPtr reg) {
    return ParsedColorTrio(reg, s);
}


template<>
inline RGBColor ParameterValue::convert(const std::string &s, ComponentRegistryPtr reg) {
    return convert<ParsedColorTrio>(s, reg).getValue();
}


template<>
inline StimulusNodePtr ParameterValue::convert(const std::string &s, ComponentRegistryPtr reg) {
    auto stimNode = reg->getStimulus(s);
    if (!stimNode) {
        throw SimpleException("Unknown stimulus", s);
    }
    return std::move(stimNode);
}


template<>
inline StimulusGroupPtr ParameterValue::convert(const std::string &s, ComponentRegistryPtr reg) {
    auto stimGroup = reg->getObject<StimulusGroup>(s);
    if (!stimGroup) {
        throw SimpleException("Unknown stimulus group", s);
    }
    return std::move(stimGroup);
}


template<>
inline boost::filesystem::path ParameterValue::convert(const std::string &s, ComponentRegistryPtr reg) {
    std::string workingPath;
    if (GlobalCurrentExperiment) {
        workingPath = GlobalCurrentExperiment->getWorkingPath();
    }
    
    // Expand path relative to the working path even if it's absolute, so that files identified via
    // client-side absolute paths are found in server-side experiment storage
    auto fullPath = expandPath(workingPath, s, true);
    
    if (!boost::filesystem::exists(fullPath)) {
        throw SimpleException("Path does not exist", fullPath.string());
    }
    
    return std::move(fullPath);
}


template<>
inline const char* ParameterValue::convert(const std::string &s, ComponentRegistryPtr reg) {
    return s.c_str();
}


END_NAMESPACE_MW


#endif























