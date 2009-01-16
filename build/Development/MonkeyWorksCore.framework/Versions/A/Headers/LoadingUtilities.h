/**
 * LoadingUtilties.h
 *
 * Copyright (c) 2004 MIT. All rights reserved.
 */

#ifndef _LOADING_UTILITIES_H
#define _LOADING_UTILITIES_H

#include <string>
#include "boost/filesystem/path.hpp"
namespace mw {
	bool loadSetupVariables();
	bool loadExperimentFromXMLParser(const boost::filesystem::path filepath);
	void parseMediaTagsFromExperiment(const boost::filesystem::path filepath);
	
	void unloadExperiment(bool announce = true);
	
	boost::filesystem::path expandPath(std::string working_directory, std::string path);
	
	
	// Some fiddling/reorganization is required here.  This function 
	// is just a stop gap...
	void prepareStimulusDisplay();
	
	void expandRangeReplicatorItems(const boost::filesystem::path filepath);
	void modifyExperimentMediaPaths(const boost::filesystem::path filepath);
	void createExperimentInstallDirectoryStructure(const std::string expName);
	
	std::string removeSpacesFromString(std::string s);
}
#endif
