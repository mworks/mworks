/**
 * PlatformDependentServices.h
 * 
 * Description: Handles platform dependent operations.
 *
 * History:
 * David Cox on 10/20/04 - Created.
 * Paul Jankunas on 03/23/05 - Added Mac OS X Scripting path.
 * Paul Jankunas on 02/15/06 - Added data file prepend/append methods.
 * 
 * Copyright 2004 MIT. All rights reserved.
 */

#ifndef _PLATFORM_DEPENDENT_SERVICES_H
#define _PLATFORM_DEPENDENT_SERVICES_H

#include "Utilities.h"
#include "Event.h"
#include <string>
#include <boost/filesystem/path.hpp>
namespace mw {
	boost::filesystem::path localPath();
	boost::filesystem::path pluginPath();
	boost::filesystem::path scriptingPath();
	boost::filesystem::path experimentInstallPath();
	boost::filesystem::path experimentStorageDirectoryName();
	
	
	// creates new memory for a path with scripting path prepended to path
	// make sure you delete the memory
	boost::filesystem::path prependScriptingPath(const std::string scriptFile);
	
	// prepends the platform dependent data path to filename
	boost::filesystem::path prependDataFilePath(const std::string filename);
	
	// appends a file extension to filename.
	std::string appendDataFileExtension(const std::string filename);
	std::string appendFileExtension(const std::string &filename, const std::string &ext);
	
	// removes all trailing characters after the systems file extension
	// specifier
	std::string removeFileExtension(const std::string file);
	std::string fileExtension(const std::string &file);
	
	
	boost::filesystem::path getLocalExperimentStorageDir(const std::string filename);
	boost::filesystem::path getLocalExperimentPath(const std::string filename);
	
	boost::filesystem::path prependLocalPath(const std::string file);
	boost::filesystem::path prependResourcePath(const std::string file);
	
	std::string appendVarFileExt(const std::string expName);
}
#endif
