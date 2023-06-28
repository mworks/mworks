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

#include <string>

#include <boost/filesystem/path.hpp>

#include "GenericData.h"


BEGIN_NAMESPACE_MW


class ExperimentUnpackager {
    
public:
    bool unpackageExperiment(const Datum &payload);
    const boost::filesystem::path & getUnpackagedExperimentPath() const { return loadedExperimentFilename; }
    
private:
    static boost::filesystem::path prependExperimentInstallPath(const std::string &expname,
                                                                const std::string &experimentFilename);
    static bool createFile(const std::string &filename, const std::string &buffer);
    
    boost::filesystem::path loadedExperimentFilename;
    
};


END_NAMESPACE_MW


#endif /* _EXPERIMENT_UNPACKAGER_H__ */
