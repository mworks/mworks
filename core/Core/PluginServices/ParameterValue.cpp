/*
 *  ParameterValue.cpp
 *  MWorksCore
 *
 *  Created by Christopher Stawarz on 3/24/11.
 *  Copyright 2011 MIT. All rights reserved.
 *
 */

#include "ParameterValue.h"

#include "Sound.h"
#include "SoundGroup.hpp"


BEGIN_NAMESPACE_MW


template<>
SoundPtr ParameterValue::convert(const std::string &s, ComponentRegistryPtr reg) {
    if (s.find_first_of('[') != std::string::npos) {
        return boost::make_shared<SoundGroupReferenceSound>(s, reg);
    }
    auto sound = reg->getObject<Sound>(s);
    if (!sound) {
        throw SimpleException("Unknown sound", s);
    }
    return sound;
}


boost::filesystem::path pathFromParameterValue(const std::string &s, bool directoryAllowed) {
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
    
    if (!directoryAllowed && boost::filesystem::is_directory(fullPath)) {
        throw SimpleException("Path is a directory", fullPath.string());
    }
    
    return fullPath;
}


END_NAMESPACE_MW






















