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






















