/*
 *  ParameterValue.cpp
 *  MWorksCore
 *
 *  Created by Christopher Stawarz on 3/24/11.
 *  Copyright 2011 MIT. All rights reserved.
 *
 */

#include "Experiment.h"
#include "ParameterValue.h"


BEGIN_NAMESPACE_MW

template <>
const char *ParameterValue::convert(const std::string& s,
                       ComponentRegistryPtr reg){
    return s.c_str();
}


template<>
VariablePtr ParameterValue::convert(const std::string &s, ComponentRegistryPtr reg) {
    return reg->getVariable(s);
}


template<>
ParsedColorTrio ParameterValue::convert(const std::string &s, ComponentRegistryPtr reg) {
    return ParsedColorTrio(reg, s);
}


template<>
RGBColor ParameterValue::convert(const std::string &s, ComponentRegistryPtr reg) {
    return convert<ParsedColorTrio>(s, reg).getValue();
}


template<>
StimulusNodePtr ParameterValue::convert(const std::string &s, ComponentRegistryPtr reg) {
    StimulusNodePtr stimNode(reg->getStimulus(s));
    
    if (!stimNode) {
        throw SimpleException("Unknown stimulus", s);
    }
    
    return stimNode;
}


template<>
StimulusGroupPtr ParameterValue::convert(const std::string &s, ComponentRegistryPtr reg) {
    StimulusGroupPtr stimGroup(reg->getObject<StimulusGroup>(s));

    if (!stimGroup) {
        throw SimpleException("Unknown stimulus group", s);
    }

    return stimGroup;
}


template<>
boost::filesystem::path ParameterValue::convert(const std::string &s, ComponentRegistryPtr reg) {
    namespace bf = boost::filesystem;
    
    std::string workingPath;
    if (GlobalCurrentExperiment) {
        workingPath = GlobalCurrentExperiment->getWorkingPath();
    }
    
    bf::path fullPath(expandPath(workingPath, s));
    
    if (!bf::exists(fullPath)) {
        throw SimpleException("Path does not exist", fullPath.string());
    }
    
    return fullPath;
}


END_NAMESPACE_MW






















