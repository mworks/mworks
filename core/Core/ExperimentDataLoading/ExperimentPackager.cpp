/**
 * ExperimentPackager.cpp
 *
 * History:
 * paul on 1/25/06 - Created.
 *
 * Copyright 2006 MIT. All rights reserved.
 */

#include "ExperimentPackager.h"

#include <fstream>
#include <iostream>

#include <boost/scope_exit.hpp>

#include "ExperimentUnpackager.h"
#include "LoadingUtilities.h"
#include "PlatformDependentServices.h"
#include "SystemEventFactory.h"
#include "Utilities.h"
#include "XMLParser.h"


BEGIN_NAMESPACE_MW


class ExperimentPackager::IncludedFilesParser : public XMLParser {
    
public:
    IncludedFilesParser(const std::string &path, IncludedFilesSet &includedFiles);
    
    void parse(bool announceProgress) override;
    
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
    
    IncludedFilesSet &includedFiles;
    
};


Datum ExperimentPackager::packageExperiment(const boost::filesystem::path &filename) {
    includedFiles.clear();
    IncludedFilesParser parser(filename.string(), includedFiles);
    try {
        parser.parse(false);
    } catch (const std::exception &e) {
        merror(M_PARSER_MESSAGE_DOMAIN, "Experiment packaging failed: %s", e.what());
        return Datum();
    }
    
    experimentFilename = XMLParser::squashFileName(filename.string());
    experimentWorkingPath = parser.getWorkingPathString();
    
    Datum eventPayload(M_DICTIONARY, M_EXPERIMENT_PACKAGE_NUMBER_ELEMENTS);
    
    // Package the experiment file
    {
        // Use getDocumentData to get the experiment file with any preprocessing and/or
        // XInclude substitutions already applied
        std::vector<xmlChar> fileData;
        parser.getDocumentData(fileData);
        
        Datum experimentContents(reinterpret_cast<char *>(fileData.data()), fileData.size());
        eventPayload.addElement(M_PACKAGER_EXPERIMENT, packageSingleFile(experimentFilename, experimentContents));
    }
    
    // Package the list of included (aka media) files
    {
        Datum mediaFilenames(M_LIST, int(includedFiles.size()));
        for (const auto &mediaFileName : includedFiles) {
            mediaFilenames.addElement(mediaFileName);
        }
        eventPayload.addElement(M_PACKAGER_MEDIA_FILENAMES, mediaFilenames);
    }
    
    return SystemEventFactory::systemEventPackage(M_SYSTEM_DATA_PACKAGE,
                                                  M_EXPERIMENT_PACKAGE,
                                                  eventPayload);
}


Datum ExperimentPackager::createMediaFilePackage(const Datum &mediaFileRequestPayload) {
    if (!(mediaFileRequestPayload.isDictionary() &&
          mediaFileRequestPayload.getNElements() == M_MEDIA_FILE_REQUEST_NUMBER_ELEMENTS &&
          mediaFileRequestPayload.getElement(M_UNPACKAGER_EXPERIMENT_FILENAME).getString() == experimentFilename))
    {
        return Datum();
    }
    
    auto mediaFilename = mediaFileRequestPayload.getElement(M_UNPACKAGER_MEDIA_FILENAME).getString();
    if (mediaFilename.empty()) {
        return Datum();
    }
    
    auto iter = includedFiles.find(mediaFilename);
    if (iter != includedFiles.end()) {
        // Remove the filename, so that it can't be requested again
        includedFiles.erase(iter);
    } else {
        // The requested file is not associated with the packaged experiment or
        // has already been requested
        return Datum();
    }
    
    auto mediaFilePackagePayload = packageSingleFile(mediaFilename, expandPath(experimentWorkingPath, mediaFilename));
    return SystemEventFactory::systemEventPackage(M_SYSTEM_DATA_PACKAGE,
                                                  M_MEDIA_FILE_PACKAGE,
                                                  mediaFilePackagePayload);
}


Datum ExperimentPackager::packageSingleFile(const std::string &filename, const boost::filesystem::path &filePath) {
    std::ifstream mediaFile(filePath.string(), std::ios::binary);
    
    // get length of file
    mediaFile.seekg(0, std::ios::end);
    auto length = mediaFile.tellg();
    
    Datum contents;
    if (length < 0) {
        // the file was never opened
        merror(M_FILE_MESSAGE_DOMAIN, "Can't find file: %s", filePath.string().c_str());
        // Leave contents undefined, which will tell the receiver that the file couldn't be opened
    } else {
        std::string buffer(length, 0);
        mediaFile.seekg(0, std::ios::beg);
        mediaFile.read(buffer.data(), buffer.size());
        contents = Datum(std::move(buffer));
    }
    
    return packageSingleFile(filename, contents);
}


Datum ExperimentPackager::packageSingleFile(const std::string &filename, const Datum &contents) {
    Datum unit(M_DICTIONARY, M_EXPERIMENT_PACKAGE_NUMBER_ELEMENTS_PER_UNIT);
    unit.addElement(M_PACKAGER_FILENAME, filename);
    unit.addElement(M_PACKAGER_CONTENTS, contents);
    return unit;
}


ExperimentPackager::IncludedFilesParser::IncludedFilesParser(const std::string &path, IncludedFilesSet &includedFiles) :
    XMLParser(path, "MWMediaPackagerTransformation.xsl"),
    includedFiles(includedFiles)
{ }


void ExperimentPackager::IncludedFilesParser::parse(bool announceProgress) {
    // Load the experiment file, applying any preprocessing and/or XInclude substitutions
    loadFile();
    
    // Look for resource declarations
    auto xpathObject = evaluateXPathExpression("//resource/@path[string-length() != 0]");
    if (xpathObject) {
        BOOST_SCOPE_EXIT( xpathObject ) {
            xmlXPathFreeObject(xpathObject);
        } BOOST_SCOPE_EXIT_END
        
        if (xpathObject->type == XPATH_NODESET && xpathObject->nodesetval && xpathObject->nodesetval->nodeNr > 0) {
            // Found one or more resource declarations, so add the identified files and directories to
            // the manifest
            for (int nodeIndex = 0; nodeIndex < xpathObject->nodesetval->nodeNr; nodeIndex++) {
                auto path = _getContent(xpathObject->nodesetval->nodeTab[nodeIndex]);
                if (boost::filesystem::is_directory(expandPath(getWorkingPathString(), path))) {
                    addDirectory(path, true);  // Recursive
                } else {
                    includedFiles.insert(path);
                }
            }
            // Return without parsing the file.  This allows experiments that declare resources to use
            // run-time expressions in "path" and other attributes that would otherwise need to be
            // evaluated at parse time.
            return;
        }
    }
    
    // No resource declarations found, so parse the experiment (expanding any replicators), and
    // infer the included files from component attributes
    XMLParser::parse(announceProgress);
}


void ExperimentPackager::IncludedFilesParser::_processCreateDirective(xmlNode *node) {
    auto child = node->children;
    while (child) {
        const std::string name(reinterpret_cast<const char *>(child->name));
        if (name == "path") {
            includedFiles.emplace(_getContent(child));
        } else if (name == "directory_path") {
            addDirectory(_getContent(child), false);  // Not recursive
        }
        child = child->next;
    }
}


void ExperimentPackager::IncludedFilesParser::addDirectory(const std::string &directoryPath, bool recursive) {
    std::vector<std::string> filePaths;
    getFilePaths(getWorkingPathString(), directoryPath, filePaths, recursive);
    includedFiles.insert(filePaths.begin(), filePaths.end());
}


END_NAMESPACE_MW
