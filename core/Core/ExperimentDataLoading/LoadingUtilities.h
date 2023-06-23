/**
 * LoadingUtilties.h
 *
 * Copyright (c) 2004 MIT. All rights reserved.
 */

#ifndef _LOADING_UTILITIES_H
#define _LOADING_UTILITIES_H

#include <string>
#include "boost/filesystem/path.hpp"


BEGIN_NAMESPACE_MW


	bool loadSetupVariables();
	bool loadExperimentFromXMLParser(const boost::filesystem::path filepath);
	
	void unloadExperiment(bool announce = true);
	
	boost::filesystem::path expandPath(std::string working_directory,
                                       std::string path,
                                       bool expandAbsolutePath = false);
	
	
	void createExperimentInstallDirectoryStructure(const std::string expName);
	
    void getFilePaths(const std::string &workingPath,
                      const std::string &directoryPath,
                      std::vector<std::string> &filePaths,
                      bool recursive = false);
    void getFilePaths(const std::string &directoryPath, std::vector<std::string> &filePaths, bool recursive = false);

    std::size_t getMatchingFilenames(const std::string &workingPath,
                                     const string &globPattern,
                                     std::vector<std::string> &filenames);


END_NAMESPACE_MW


#endif
