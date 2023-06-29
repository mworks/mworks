/*
 *  ExperimentUnpackager.h
 *  MWorksCore
 *
 *  Created by Ben Kennedy on 1/5/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _EXPERIMENT_UNPACKAGER_H__
#define _EXPERIMENT_UNPACKAGER_H__

#include <deque>
#include <string>

#include <boost/filesystem/path.hpp>

#include "GenericData.h"

#define M_UNPACKAGER_EXPERIMENT_FILENAME "Experiment Filename"
#define M_UNPACKAGER_MEDIA_FILENAME "Media Filename"

#define M_MEDIA_FILE_REQUEST_NUMBER_ELEMENTS 2


BEGIN_NAMESPACE_MW


class ExperimentUnpackager {
    
public:
    bool unpackageExperiment(const Datum &payload);
    const boost::filesystem::path & getExperimentFilePath() const { return experimentFilePath; }
    
    Datum createMediaFileRequest() const;
    bool unpackageMediaFile(const Datum &mediaFilePackagePayload);
    
private:
    static boost::filesystem::path prependExperimentInstallPath(const std::string &experimentName,
                                                                const std::string &experimentFilename);
    static bool createFile(const boost::filesystem::path &filePath, const Datum &contents);
    
    std::string experimentFilename;
    boost::filesystem::path experimentFilePath;
    boost::filesystem::path experimentInstallDirectoryPath;
    std::deque<std::string> requiredMediaFilenames;
    
};


END_NAMESPACE_MW


#endif /* _EXPERIMENT_UNPACKAGER_H__ */
