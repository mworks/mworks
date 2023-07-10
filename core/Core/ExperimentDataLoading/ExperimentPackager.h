/*!
 * @header ExperimentPackager.h
 *
 * @discussion Controls the packaging and unpackaging of experiment files and
 * all the media files that it references.
 *
 * History:<BR/>
 * paul on 1/25/06 - Created.<BR/>
 * 
 * @copyright MIT
 * @updated 1/25/06
 * @version 1.0.0
 */

#ifndef _EXPERIMENT_PACKAGER_H__
#define _EXPERIMENT_PACKAGER_H__

#include <string>
#include <unordered_set>

#include <boost/filesystem/path.hpp>

#include "GenericData.h"

#define M_PACKAGER_FILENAME "Filename"
#define M_PACKAGER_CONTENTS "Contents"
#define M_PACKAGER_EXPERIMENT "Experiment"
#define M_PACKAGER_MEDIA_FILENAMES "Media Filenames"

#define M_EXPERIMENT_PACKAGE_NUMBER_ELEMENTS_PER_UNIT 2
#define M_EXPERIMENT_PACKAGE_NUMBER_ELEMENTS 2


BEGIN_NAMESPACE_MW


class ExperimentPackager {
    
    class IncludedFilesParser;
    
public:
    Datum packageExperiment(const boost::filesystem::path &fileWithFullPath);
    Datum createMediaFilePackage(const Datum &mediaFileRequestPayload);
    
private:
    static Datum packageSingleFile(const std::string &filename, const boost::filesystem::path &filePath);
    static Datum packageSingleFile(const std::string &filename, const Datum &contents);
    
    std::string experimentFilename;
    std::string experimentWorkingPath;
    
    using IncludedFilesSet = std::unordered_set<std::string>;
    IncludedFilesSet includedFiles;
    
};


END_NAMESPACE_MW


#endif /* _EXPERIMENT_PACKAGER_H__ */
