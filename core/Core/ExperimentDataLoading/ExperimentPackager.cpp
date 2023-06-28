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

#include "LoadingUtilities.h"
#include "PlatformDependentServices.h"
#include "SystemEventFactory.h"
#include "Utilities.h"


BEGIN_NAMESPACE_MW


Datum ExperimentPackager::packageExperiment(const boost::filesystem::path &filename) {
    IncludedFilesParser parser(filename.string());
    std::string working_path_string;
    Datum include_files;
    
    try {
        parser.parse(false);
        working_path_string = parser.getWorkingPathString();
        include_files = parser.getIncludedFilesManifest();
    } catch (const std::exception &e) {
        merror(M_PARSER_MESSAGE_DOMAIN, "Experiment packaging failed: %s", e.what());
        return Datum();
    }
    
    Datum eventPayload(M_DICTIONARY, M_EXPERIMENT_PACKAGE_NUMBER_ELEMENTS);
    
    {
        // Use getDocumentData to get the experiment file with any preprocessing and/or
        // XInclude substitutions already applied
        std::vector<xmlChar> fileData;
        parser.getDocumentData(fileData);
        
        Datum contents(reinterpret_cast<char *>(&(fileData.front())), fileData.size());
        eventPayload.addElement(M_PACKAGER_EXPERIMENT_STRING,
                                packageSingleFile(XMLParser::squashFileName(filename.string()), contents));
    }
    
    {
        Datum mediaFilesPayload(M_LIST, include_files.getNElements());
        
        for (const auto &item : include_files.getList()) {
            auto &mediaName = item.getString();
            auto mediaPath = expandPath(working_path_string, mediaName);
            auto mediaElement = packageSingleFile(mediaName, mediaPath);
            if (mediaElement.isUndefined()) {
                merror(M_FILE_MESSAGE_DOMAIN, "Can't find file: %s", mediaPath.string().c_str());
                return Datum();
            }
            mediaFilesPayload.addElement(mediaElement);
        }
        
        eventPayload.addElement(M_PACKAGER_MEDIA_BUFFERS_STRING, mediaFilesPayload);
    }
    
    return SystemEventFactory::systemEventPackage(M_SYSTEM_DATA_PACKAGE,
                                                  M_EXPERIMENT_PACKAGE,
                                                  eventPayload);
}


Datum ExperimentPackager::packageSingleFile(const std::string &filename, const boost::filesystem::path &filepath) {
    std::ifstream mediaFile(filepath.string(), std::ios::binary);
    
    // get length of file:
    mediaFile.seekg(0, std::ios::end);
    auto length = mediaFile.tellg();
    // if the file was never opened
    if (length < 0) {
        return Datum();
    }
    
    std::string buffer(length, 0);
    mediaFile.seekg(0, std::ios::beg);
    mediaFile.read(buffer.data(), buffer.size());
    
    return packageSingleFile(filename, Datum(std::move(buffer)));
}


Datum ExperimentPackager::packageSingleFile(const std::string &filename, const Datum &contents) {
    Datum unit(M_DICTIONARY, M_EXPERIMENT_PACKAGE_NUMBER_ELEMENTS_PER_UNIT);
    unit.addElement(M_PACKAGER_FILENAME_STRING, filename);
    unit.addElement(M_PACKAGER_CONTENTS_STRING, contents);
    return unit;
}


IncludedFilesParser::IncludedFilesParser(const std::string &path) :
    XMLParser(path, "MWMediaPackagerTransformation.xsl"),
    manifest(Datum::list_value_type{})
{ }


void IncludedFilesParser::parse(bool announceProgress) {
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
                    manifest.addElement(path);
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


void IncludedFilesParser::_processCreateDirective(xmlNode *node) {
    auto child = node->children;
    while (child) {
        const std::string name(reinterpret_cast<const char *>(child->name));
        if (name == "path") {
            manifest.addElement(_getContent(child));
        } else if (name == "directory_path") {
            addDirectory(_getContent(child), false);  // Not recursive
        }
        child = child->next;
    }
}


void IncludedFilesParser::addDirectory(const std::string &directoryPath, bool recursive) {
    std::vector<std::string> filePaths;
    getFilePaths(getWorkingPathString(), directoryPath, filePaths, recursive);
    for (const auto &path : filePaths) {
        manifest.addElement(path);
    }
}


END_NAMESPACE_MW
