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
#include <boost/filesystem/operations.hpp>
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
	const char * EXPERIMENT_TEMPORARY_DIR_NAME = "tmp";
	const char * VARIABLE_FILE_EXT = "_var.xml";
	const char * EXPERIMENT_STORAGE_DIR_NAME = "Experiment Storage";
	const char * SAVED_VARIABLES_DIR_NAME = "Saved Variables";
	const char * SETUP_VARIABLES_FILENAME = "setup_variables.xml";
	
	boost::filesystem::path pluginPath() {
		return boost::filesystem::path(PLUGIN_PATH);
	}
	
	boost::filesystem::path scriptingPath() {
		return boost::filesystem::path(SCRIPTING_PATH);
	}
	
	boost::filesystem::path dataFilePath() {
		namespace bf = boost::filesystem;
        bf::path data_file_path;
        
        NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
        
        NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
        if (paths && [paths count]) {
            data_file_path = (bf::path([[paths objectAtIndex:0] UTF8String]) /
                              bf::path(DATA_FILE_PATH));
        }
        
        [pool drain];
        
		return data_file_path;
	}
	
	boost::filesystem::path userPath() {
		namespace bf = boost::filesystem;
        bf::path user_path;
        
        NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

        NSArray *paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
        if (paths && [paths count]) {
            user_path = bf::path([[paths objectAtIndex:0] UTF8String]) / bf::path(CONFIG_PATH);
        }
        
        [pool drain];

		return user_path;
	}
	
	boost::filesystem::path localPath() {
		namespace bf = boost::filesystem;
		return bf::path("/Library/Application Support") / bf::path(CONFIG_PATH);
	}
	
    boost::filesystem::path setupVariablesFile() {
        namespace bf = boost::filesystem;
        bf::path setupPath(prependUserPath(SETUP_VARIABLES_FILENAME));
        if (setupPath.empty() || !bf::is_regular_file(setupPath)) {
            setupPath = prependLocalPath(SETUP_VARIABLES_FILENAME);
        }
        return setupPath;
    }
	
	boost::filesystem::path resourcePath() {
		std::string resource_path;
		
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
		for(NSBundle *framework in [NSBundle allFrameworks]) {
			if([[[[framework bundlePath] lastPathComponent] stringByDeletingPathExtension] isEqualToString:@"MWorksCore"]) {
				resource_path = [[framework resourcePath] cStringUsingEncoding:NSASCIIStringEncoding];
			}
		}
		
		[pool drain];
								   
		return boost::filesystem::path(resource_path);
	}
	
	boost::filesystem::path experimentInstallPath() {
		namespace bf = boost::filesystem;
        
        NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
		bf::path experiment_install_path(bf::path([NSTemporaryDirectory() UTF8String]) /
                                         bf::path(EXPERIMENT_INSTALL_PATH));
        [pool drain];
        
        return experiment_install_path;
	}
	
	boost::filesystem::path experimentStorageDirectoryName() {
		return boost::filesystem::path(EXPERIMENT_TEMPORARY_DIR_NAME);
	}
	
	boost::filesystem::path prependScriptingPath(const std::string scriptFile){
		return scriptingPath() / boost::filesystem::path(scriptFile);
	}
	
	boost::filesystem::path prependUserPath(const std::string file){
		return userPath() / boost::filesystem::path(file);
	}
	
	boost::filesystem::path prependLocalPath(const std::string file){
		return localPath() / boost::filesystem::path(file);
	}
	
	boost::filesystem::path prependResourcePath(const std::string file){
		return resourcePath() / boost::filesystem::path(file);
	}
	
	boost::filesystem::path prependDataFilePath(const std::string filename) {
		namespace bf = boost::filesystem;
		return dataFilePath() / bf::path(filename);
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
		
		return getLocalExperimentPath(expName) / experimentStorageDirectoryName();
		
	}
	
	boost::filesystem::path getLocalExperimentPath(const std::string expName) {
		namespace bf = boost::filesystem;
		
		return experimentInstallPath() / bf::path(expName);
	}
    
    boost::filesystem::path getExperimentSavedVariablesPath(const std::string expName) {
		namespace bf = boost::filesystem;
        return (dataFilePath().parent_path() /
                bf::path(EXPERIMENT_STORAGE_DIR_NAME) /
                bf::path(expName) /
                bf::path(SAVED_VARIABLES_DIR_NAME));
    }
	
	std::string appendVarFileExt(const std::string expName) {
		return expName + VARIABLE_FILE_EXT;
	}
}
