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
    if (!(payload.isDictionary() &&
          payload.getNElements() == M_EXPERIMENT_PACKAGE_NUMBER_ELEMENTS))
    {
        merror(M_NETWORK_MESSAGE_DOMAIN, "Invalid payload type for experiment package");
        return false;
    }
    
    auto experimentFilePackage = payload.getElement(M_PACKAGER_EXPERIMENT);
    if (!(experimentFilePackage.isDictionary() &&
          experimentFilePackage.getNElements() == M_EXPERIMENT_PACKAGE_NUMBER_ELEMENTS_PER_UNIT))
    {
        return false;
    }
    
    experimentFilename = experimentFilePackage.getElement(M_PACKAGER_FILENAME).getString();
    if (experimentFilename.empty()) {
        return false;
    }
    
    experimentFilePath = prependExperimentInstallPath(removeFileExtension(experimentFilename), experimentFilename);
    experimentInstallDirectoryPath = experimentFilePath.parent_path();
    createExperimentInstallDirectoryStructure(experimentFilename);
    
    // create the XML file
    auto experimentContents = experimentFilePackage.getElement(M_PACKAGER_CONTENTS);
    if (!(experimentContents.isString() &&
          experimentContents.getSize() > 0 &&
          createFile(experimentFilePath, experimentContents)))
    {
        return false;
    }
    
    // get the list of media filenames to be requested
    auto mediaFilenames = payload.getElement(M_PACKAGER_MEDIA_FILENAMES);
    if (mediaFilenames.isList()) {
        for (auto &mediaFilename : mediaFilenames.getList()) {
            requiredMediaFilenames.emplace_back(mediaFilename);
        }
    }
    
    return true;
}


Datum ExperimentUnpackager::createMediaFileRequest() const {
    if (requiredMediaFilenames.empty()) {
        // We've already have all required media files
        return Datum();
    }
    
    // Generate a request for the next media file
    Datum mediaFileRequestPayload(M_DICTIONARY, M_MEDIA_FILE_REQUEST_NUMBER_ELEMENTS);
    mediaFileRequestPayload.addElement(M_UNPACKAGER_EXPERIMENT_FILENAME, experimentFilename);
    mediaFileRequestPayload.addElement(M_UNPACKAGER_MEDIA_FILENAME, requiredMediaFilenames.front());
    
    return SystemEventFactory::systemEventPackage(M_SYSTEM_CONTROL_PACKAGE,
                                                  M_REQUEST_MEDIA_FILE,
                                                  mediaFileRequestPayload);
}


bool ExperimentUnpackager::unpackageMediaFile(const Datum &mediaFilePackagePayload) {
    if (!(mediaFilePackagePayload.isDictionary() &&
          mediaFilePackagePayload.getNElements() == M_EXPERIMENT_PACKAGE_NUMBER_ELEMENTS_PER_UNIT))
    {
        merror(M_FILE_MESSAGE_DOMAIN, "Incorrectly packaged media file");
        return false;
    }
    
    auto mediaFilename = mediaFilePackagePayload.getElement(M_PACKAGER_FILENAME);
    auto mediaFileContents = mediaFilePackagePayload.getElement(M_PACKAGER_CONTENTS);
    if (!(requiredMediaFilenames.size() > 0 &&
          mediaFilename.getString() == requiredMediaFilenames.front() &&
          mediaFileContents.isString()))
    {
        return false;
    }
    
    // We've received the next media file, so remove it from the list
    requiredMediaFilenames.pop_front();
    
    auto mediaFilePath = (experimentInstallDirectoryPath /
                          boost::filesystem::path(mediaFilename.getString()).relative_path());
    if (!createFile(mediaFilePath, mediaFileContents)) {
        return false;
    }
    
    return true;
}


boost::filesystem::path ExperimentUnpackager::prependExperimentInstallPath(const std::string &experimentName,
                                                                           const std::string &experimentFilename)
{
    if (experimentName.empty() || experimentFilename.empty()) {
        return boost::filesystem::path("");
    }
    
    auto exp_tmp_path = getLocalExperimentStorageDir(experimentName);
    auto exp_path = exp_tmp_path / boost::filesystem::path(experimentFilename);
    
    boost::filesystem::create_directories(exp_tmp_path);
    
    return exp_path;
}


bool ExperimentUnpackager::createFile(const boost::filesystem::path &filePath, const Datum &contents) {
    boost::filesystem::create_directories(filePath.parent_path());
    
    // Create an output file
    std::ofstream outFile(filePath.string(), std::ios::binary);
    if (outFile) {
        // Write the data
        auto buffer = contents.getString();
        outFile.write(buffer.data(), buffer.size());
        if (outFile) {
            // Flush the file
            outFile.flush();
        }
    }
    
    if (!outFile) {
        merror(M_FILE_MESSAGE_DOMAIN,
               "Failed to create server side experiment file %s",
               filePath.string().c_str());
        return false;
    }
    
    return true;
}


END_NAMESPACE_MW
