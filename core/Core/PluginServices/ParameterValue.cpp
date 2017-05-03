/*
 *  ParameterValue.cpp
 *  MWorksCore
 *
 *  Created by Christopher Stawarz on 3/24/11.
 *  Copyright 2011 MIT. All rights reserved.
 *
 */

#include "ParameterValue.h"


BEGIN_NAMESPACE_MW


boost::filesystem::path pathFromParameterString(const std::string &s) {
    std::string workingPath;
    if (auto experiment = GlobalCurrentExperiment) {
        workingPath = experiment->getWorkingPath();
    }
    
    // Expand path relative to the working path even if it's absolute, so that files identified via
    // client-side absolute paths are found in server-side experiment storage
    auto fullPath = expandPath(workingPath, s, true);
    
    if (!boost::filesystem::exists(fullPath)) {
        throw SimpleException("Path does not exist", fullPath.string());
    }
    
    return fullPath;
}


END_NAMESPACE_MW






















