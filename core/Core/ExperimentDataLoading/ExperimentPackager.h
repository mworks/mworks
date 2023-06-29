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
#include "XMLParser.h"

#define M_PACKAGER_FILENAME_STRING "Filename"
#define M_PACKAGER_CONTENTS_STRING "Contents"
#define M_PACKAGER_EXPERIMENT_STRING "Experiment"
#define M_PACKAGER_MEDIA_BUFFERS_STRING "Media Buffers"

#define M_PACKAGER_FULL_NAME "full_path"
#define M_PACKAGER_RELATIVE_NAME "relative_path"

#define M_EXPERIMENT_PACKAGE_NUMBER_ELEMENTS_PER_UNIT 2
#define M_EXPERIMENT_PACKAGE_NUMBER_ELEMENTS 2


BEGIN_NAMESPACE_MW


class ExperimentPackager {
    
public:
    Datum packageExperiment(const boost::filesystem::path &fileWithFullPath);
    
private:
    Datum packageSingleFile(const std::string &filename, const boost::filesystem::path &filepath);
    Datum packageSingleFile(const std::string &filename, const Datum &contents);
    
};


class IncludedFilesParser : public XMLParser {
    
public:
    explicit IncludedFilesParser(const std::string &path);
    
    void parse(bool announceProgress) override;
    const std::unordered_set<std::string> & getIncludedFiles() const { return includedFiles; }
    
private:
    // instead of building experiment, just look for path arguments and save
    // the results
    void _processCreateDirective(xmlNode *node) override;
    void _processAnonymousCreateDirective(xmlNode *node) override { _processCreateDirective(node); }
    
    // don't do anything for these
    void _processConnectDirective(xmlNode *node) override { }
    void _processInstanceDirective(xmlNode *node) override { }
    void _processFinalizeDirective(xmlNode *node) override { }
    
    void addDirectory(const std::string &directoryPath, bool recursive);
    
    std::unordered_set<std::string> includedFiles;
    
};


END_NAMESPACE_MW


#endif /* _EXPERIMENT_PACKAGER_H__ */
