/**
 * PlatformDependentServices.cpp
 *
 * History:
 * David Cox on 10/20/04 - Created.
 * Paul Jankunas on 03/23/05 - Changed scriptingPath function to return
 *                  a standardized Mac OS X path.
 * 
 * Copyright 2004 MIT. All rights reserved.
 */

#include "PlatformDependentServices.h"
#include <boost/filesystem/path.hpp>
#ifdef	__APPLE__
#include <Cocoa/Cocoa.h>
#endif

namespace mw {
	
#ifdef __APPLE__
    const char * DATA_FILE_PATH = "MWorks/Data";
	const char * PLUGIN_PATH = "/Library/Application Support/MWorks/Plugins/Core";
	const char * SCRIPTING_PATH = "/Library/Application Support/MWorks/Scripting";
	const char * CONFIG_PATH = "MWorks/Configuration";
	const char * EXPERIMENT_INSTALL_PATH = "MWorks/Experiment Cache";
#else
	
    const char * DATA_FILE_PATH "/usr/local/MWorks/plugins/";
	const char * PLUGIN_PATH = "/usr/local/MWorks/plugins/";
	const char * SCRIPTING_PATH = "/usr/local/MWorks/scripting/";
	const char * CONFIG_PATH = "/usr/local/MWorks/local/";
	const char * EXPERIMENT_INSTALL_PATH = "/tmp/mw_experiments/";
#endif
	
	const char * DATA_FILE_EXT = ".mwk";
	const char * EXPERIMENT_STORAGE_DIRECTORY_NAME = "tmp";
	const char * VARIABLE_FILE_EXT = "_var.xml";
	
	boost::filesystem::path pluginPath() {
		return boost::filesystem::path(PLUGIN_PATH, boost::filesystem::native);
	}
	
	boost::filesystem::path scriptingPath() {
		return boost::filesystem::path(SCRIPTING_PATH, boost::filesystem::native);
	}
	
	boost::filesystem::path dataFilePath() {
		namespace bf = boost::filesystem;
        
        NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
        if (!paths || ![paths count])
            return bf::path();
        
		return bf::path([[paths objectAtIndex:0] UTF8String], bf::native) / bf::path(DATA_FILE_PATH, bf::native);
	}
	
	boost::filesystem::path userPath() {
		namespace bf = boost::filesystem;

        NSArray *paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
        if (!paths || ![paths count])
            return bf::path();

		return bf::path([[paths objectAtIndex:0] UTF8String], bf::native) / bf::path(CONFIG_PATH, bf::native);
	}
	
	boost::filesystem::path localPath() {
		namespace bf = boost::filesystem;
		return bf::path("/Library/Application Support", bf::native) / bf::path(CONFIG_PATH, bf::native);
	}
	
	boost::filesystem::path resourcePath() {
		std::string resource_path;
		
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
		for(NSBundle *framework in [NSBundle allFrameworks]) {
			if([[[[framework bundlePath] lastPathComponent] stringByDeletingPathExtension] isEqualToString:@"MWorksCore"]) {
				resource_path = [[framework resourcePath] cStringUsingEncoding:NSASCIIStringEncoding];
			}
		}
		
		[pool release];
								   
		return boost::filesystem::path(resource_path, boost::filesystem::native);
	}
	
	boost::filesystem::path experimentInstallPath() {
		namespace bf = boost::filesystem;
		return (bf::path([NSTemporaryDirectory() UTF8String], bf::native) /
                bf::path(EXPERIMENT_INSTALL_PATH, bf::native));
	}
	
	boost::filesystem::path experimentStorageDirectoryName() {
		return boost::filesystem::path(EXPERIMENT_STORAGE_DIRECTORY_NAME, boost::filesystem::native);
	}
	
	boost::filesystem::path prependScriptingPath(const std::string scriptFile){
		return scriptingPath() / boost::filesystem::path(scriptFile, boost::filesystem::native);
	}
	
	boost::filesystem::path prependUserPath(const std::string file){
		return userPath() / boost::filesystem::path(file, boost::filesystem::native);
	}
	
	boost::filesystem::path prependLocalPath(const std::string file){
		return localPath() / boost::filesystem::path(file, boost::filesystem::native);
	}
	
	boost::filesystem::path prependResourcePath(const std::string file){
		return resourcePath() / boost::filesystem::path(file, boost::filesystem::native);
	}
	
	boost::filesystem::path prependDataFilePath(const std::string filename) {
		namespace bf = boost::filesystem;
		return dataFilePath() / bf::path(filename, bf::native);
	}
	
	std::string appendDataFileExtension(const std::string filename_) {
		return appendFileExtension(filename_, DATA_FILE_EXT);
	}
	
	std::string appendFileExtension(const std::string &filename_, const std::string &ext_) {
		std::string filename(filename_);
		std::string ext(ext_);
		
		// check to see if there already is an extension
		if(filename.length() <= ext.length() || filename.substr(filename.length()-ext.length()) != ext) {
			filename.append(ext);
		}	
		
		return filename;		
	}
	
	std::string fileExtension(const std::string &file) {
		return file.substr(file.rfind(".")+1, file.length()-file.rfind(".")-1);
	}
	
	std::string removeFileExtension(const std::string file) {
		return file.substr(0,file.rfind("."));
	}
	
	boost::filesystem::path getLocalExperimentStorageDir(const std::string expName) {
		namespace bf = boost::filesystem;
		
		return getLocalExperimentPath(expName) / 
		bf::path(EXPERIMENT_STORAGE_DIRECTORY_NAME, bf::native);
		
	}
	
	boost::filesystem::path getLocalExperimentPath(const std::string expName) {
		namespace bf = boost::filesystem;
		
		return experimentInstallPath() / bf::path(expName, bf::native);
	}
	
	std::string appendVarFileExt(const std::string expName) {
		return expName + VARIABLE_FILE_EXT;
	}
}
