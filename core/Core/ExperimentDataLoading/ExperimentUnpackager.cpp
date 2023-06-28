/*
 *  ExperimentUnpackager.cpp
 *  MWorksCore
 *
 *  Created by Ben Kennedy on 1/5/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "ExperimentUnpackager.h"

#include <fstream>
#include <iostream>

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/path.hpp>

#include "ExperimentPackager.h"
#include "LoadingUtilities.h"
#include "PlatformDependentServices.h"
#include "SystemEventFactory.h"
#include "Utilities.h"


BEGIN_NAMESPACE_MW


bool ExperimentUnpackager::unpackageExperiment(const Datum &payload) {
    if (!payload.isDictionary()) {
        merror(M_NETWORK_MESSAGE_DOMAIN, "Invalid payload type for experiment package");
        return false;
    }
    
    auto experimentFilePackage = payload.getElement(M_PACKAGER_EXPERIMENT_STRING);
    if (!(experimentFilePackage.isDictionary() &&
          experimentFilePackage.getNElements() == M_EXPERIMENT_PACKAGE_NUMBER_ELEMENTS_PER_UNIT))
    {
        return false;
    }
    
    auto experimentFileName = experimentFilePackage.getElement(M_PACKAGER_FILENAME_STRING);
    if (!(experimentFileName.isString() && experimentFileName.getSize() > 0)) {
        return false;
    }
    
    auto &experimentName = experimentFileName.getString();
    loadedExperimentFilename = prependExperimentInstallPath(removeFileExtension(experimentName), experimentName);
    auto experimentPath = loadedExperimentFilename.parent_path().string();
    createExperimentInstallDirectoryStructure(experimentName);
    
    // create the XML file
    auto experimentFileBuffer = experimentFilePackage.getElement(M_PACKAGER_CONTENTS_STRING);
    if (!(experimentFileBuffer.isString() && experimentFileBuffer.getSize() > 0)) {
        return false;
    }
    if (!createFile(loadedExperimentFilename.string(), experimentFileBuffer.getString())) {
        // failed to create experiment file
        merror(M_FILE_MESSAGE_DOMAIN,
               "Failed to create server side experiment file %s",
               loadedExperimentFilename.string().c_str());
        return false;
    }
    
    // create all of the other media files
    auto mediaFileList = payload.getElement(M_PACKAGER_MEDIA_BUFFERS_STRING);
    if (mediaFileList.isList()) {
        for (auto &mediaFilePackage : mediaFileList.getList()) {
            if (!(mediaFilePackage.isDictionary() && mediaFilePackage.getNElements() == 2)) {
                merror(M_FILE_MESSAGE_DOMAIN, "incorrectly packaged media files");
                return false;
            }
            
            auto mediaFileName = mediaFilePackage.getElement(M_PACKAGER_FILENAME_STRING);
            auto mediaFileBuffer = mediaFilePackage.getElement(M_PACKAGER_CONTENTS_STRING);
            if (!(mediaFileName.isString() && mediaFileName.getSize() > 0 && mediaFileBuffer.isString())) {
                return false;
            }
            
            auto filenameWPath = experimentPath + "/" + mediaFileName.getString();
            if (!createFile(filenameWPath, mediaFileBuffer.getString())) {
                // failed to create experiment file
                merror(M_FILE_MESSAGE_DOMAIN,
                       "Failed to create server side experiment file %s",
                       filenameWPath.c_str());
                return false;
            }
        }
    }
    
    return true;
}


boost::filesystem::path ExperimentUnpackager::prependExperimentInstallPath(const std::string &expname,
                                                                           const std::string &experimentFilename)
{
    if (expname.empty() || experimentFilename.empty()) {
        return boost::filesystem::path("");
    }
    
    auto exp_tmp_path = getLocalExperimentStorageDir(expname);
    auto exp_path = exp_tmp_path / boost::filesystem::path(experimentFilename);
    
    boost::filesystem::create_directories(exp_tmp_path);
    
    return exp_path;
}


bool ExperimentUnpackager::createFile(const std::string &filename, const std::string &buffer) {
    boost::filesystem::path filePath(filename);
    boost::filesystem::create_directories(filePath.parent_path());
    
    // create an output file.
    std::ofstream outFile(filePath.string(), std::ios::binary);
    
    //write the data
    outFile.write(buffer.data(), buffer.size());
    
    // flush buffer
    outFile.flush();
    
    return true;
}


END_NAMESPACE_MW
