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
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#include <boost/lexical_cast.hpp>
#pragma clang diagnostic pop

#include "ComponentRegistry.h"
#include "Experiment.h"
#include "Map.h"
#include "ParsedColorTrio.h"
#include "Stimulus.h"


BEGIN_NAMESPACE_MW


class Sound;


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
    return stimNode;
}


template<>
inline StimulusGroupPtr ParameterValue::convert(const std::string &s, ComponentRegistryPtr reg) {
    auto stimGroup = reg->getObject<StimulusGroup>(s);
    if (!stimGroup) {
        throw SimpleException("Unknown stimulus group", s);
    }
    return stimGroup;
}


template<>
boost::shared_ptr<Sound> ParameterValue::convert(const std::string &s, ComponentRegistryPtr reg);


template<>
SelectionType ParameterValue::convert(const std::string &s, ComponentRegistryPtr reg);


template<>
SampleType ParameterValue::convert(const std::string &s, ComponentRegistryPtr reg);


boost::filesystem::path pathFromParameterValue(const std::string &s, bool directoryAllowed = false);


inline boost::filesystem::path pathFromParameterValue(const VariablePtr &expr, bool directoryAllowed = false) {
    return pathFromParameterValue(expr->getValue().getString(), directoryAllowed);
}


template<>
inline boost::filesystem::path ParameterValue::convert(const std::string &s, ComponentRegistryPtr reg) {
    return pathFromParameterValue(s, true);  // Allow directory for compatibility with existing code
}


template<>
inline const char* ParameterValue::convert(const std::string &s, ComponentRegistryPtr reg) {
    return s.c_str();
}


inline VariablePtr parsedText(const ParameterValue &param) {
    return param.getRegistry()->getParsedString(param.str());
}


inline VariablePtr variableOrText(const ParameterValue &param) {
    try {
        return VariablePtr(param);
    } catch (const SimpleException &) {
        return parsedText(param);
    }
}


inline VariablePtr optionalVariable(const ParameterValue &param) {
    if (param.empty()) {
        return VariablePtr();
    }
    return VariablePtr(param);
}


inline VariablePtr optionalVariableOrText(const ParameterValue &param) {
    if (param.empty()) {
        return VariablePtr();
    }
    return variableOrText(param);
}


END_NAMESPACE_MW


#endif
