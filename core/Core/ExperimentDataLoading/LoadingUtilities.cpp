/**
 * LoadingUtilities.cpp
 * 
 * Copyright (c) 2004 MIT. All rights reserved.
 */


#include "OpenGLContextManager.h"
#include "Event.h"
#include "LoadingUtilities.h"
#include "Experiment.h"
#include "PlatformDependentServices.h"
#include "SystemEventFactory.h"
#include "EventBuffer.h"
#include "StandardVariables.h"
#include "StandardStimuli.h"
#include <string>
#include <boost/filesystem/directory.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/pointer_cast.hpp>
#include "ComponentRegistry.h"
#include "XMLParser.h"
#include "DataFileManager.h"
#include <glob.h>
#include <boost/scope_exit.hpp>


BEGIN_NAMESPACE_MW


	bool loadSetupVariables() {
		boost::filesystem::path setupPath(setupVariablesFile());
		shared_ptr<ComponentRegistry> reg = ComponentRegistry::getSharedRegistry();
		XMLParser parser(reg, setupPath.string());
		parser.parse();
		return true;
	}
	
	bool loadExperimentFromXMLParser(const boost::filesystem::path filepath) {
		const std::string substitutionDescriptor("%s");
		
		
		try {
			unloadExperiment(false); // delete an experiment if one exists.
		} catch(std::exception& e){
			merror(M_PARSER_MESSAGE_DOMAIN, "%s", e.what());
			GlobalCurrentExperiment = shared_ptr<Experiment>();
			global_outgoing_event_buffer->putEvent(SystemEventFactory::currentExperimentState());
			return false;
		}
				
		shared_ptr<ComponentRegistry> reg = ComponentRegistry::getSharedRegistry();
        std::vector<xmlChar> fileData;
		
		try {
			XMLParser parser(reg, filepath.string());
			
			parser.parse(true);
            
            // Use getDocumentData to get the experiment file with any preprocessing and/or
            // XInclude substitutions already applied
            parser.getDocumentData(fileData);
			
		} catch(SimpleException& e){
            // This is the "main" catch block for parsing
			display_extended_error_information(e);
            unloadExperiment(false);
		} catch(std::exception& e){
            merror(M_PARSER_MESSAGE_DOMAIN, 
                  "An unanticipated error occurred.  This is probably a bug, and someone will want to run this in a debugger.  Error message was: \"%s\"",
                  e.what());
            unloadExperiment(false);
        }
        
        auto experiment = GlobalCurrentExperiment;
		if (!experiment) {
			global_outgoing_event_buffer->putEvent(SystemEventFactory::currentExperimentState());
			return false;
		}
        experiment->setExperimentPath(filepath.parent_path().parent_path().string());
        
        // At this point, all displays are already cleared.  However, by clearing them again now, *after* all the
        // stimuli have been created and (probably) loaded, we can "commit" any OpenGL and/or Metal actions
        // that have been taken.  While it seems like this shouldn't matter much, it appears to have a definite,
        // beneficial influence on subsequent display-update performance, particularly on iOS.
        experiment->clearStimulusDisplays();
        
        // Store the XML source of the experiment in #loadedExperiment, so that it will be
        // recorded in the event stream (unless the experiment itself has already set #loadedExperiment,
        // in which case leave it unchanged)
        if (loadedExperiment->getValue().getSize() == 0) {
            loadedExperiment->setValue(Datum(reinterpret_cast<char *>(fileData.data()), fileData.size()));
        }
		
		global_outgoing_event_buffer->putEvent(SystemEventFactory::componentCodecPackage());
		global_outgoing_event_buffer->putEvent(SystemEventFactory::codecPackage());
		global_outgoing_event_buffer->putEvent(SystemEventFactory::currentExperimentState());
		global_outgoing_event_buffer->putEvent(SystemEventFactory::protocolPackage());
		global_variable_registry->announceAll();
		
		return true;
	}
	
	void unloadExperiment(bool announce) {
		
		GlobalCurrentExperiment = shared_ptr<Experiment>();
		
		shared_ptr<ComponentRegistry> component_registry = ComponentRegistry::getSharedRegistry();
		component_registry->resetInstances();
		
		if(global_variable_registry != NULL) {	// exp. already loaded
			global_variable_registry->reset();
            //global_variable_registry = shared_ptr<VariableRegistry>(new VariableRegistry(global_outgoing_event_buffer));
			initializeStandardVariables(global_variable_registry);
			loadSetupVariables();
		}
		
        
        shared_ptr<OpenGLContextManager> opengl_context_manager = OpenGLContextManager::instance(false);
        if(opengl_context_manager != NULL){
            opengl_context_manager->releaseContexts();
            //opengl_context_manager->destroy();
        }
		
        if (auto dataFileManager = DataFileManager::instance(false)) {
            dataFileManager->closeFile();
            dataFileManager->clearAutoOpenFilename();
        }
		
		if(announce){
			
			global_outgoing_event_buffer->putEvent(SystemEventFactory::componentCodecPackage());
			global_outgoing_event_buffer->putEvent(SystemEventFactory::codecPackage());
			
			global_outgoing_event_buffer->putEvent(SystemEventFactory::currentExperimentState());
			global_variable_registry->announceAll();
		}
	}
	
	boost::filesystem::path expandPath(std::string working_directory, std::string path, bool expandAbsolutePath) {
		
		boost::filesystem::path specific_path;
		boost::filesystem::path full_path;
		
		try {
			specific_path = boost::filesystem::path(path);
		} catch(std::exception& e){
			cerr << "bad path" << endl;
		}
		
		if (path[0] == '/' && !expandAbsolutePath) {
			return specific_path;
		} else {
			try{
				
				boost::filesystem::path working_path(working_directory);
				full_path = working_path / specific_path;
			} catch(std::exception& e){
				cerr << "bad path" << endl;
			}
			
			return full_path;
		}
		
	}
	
	
	void createExperimentInstallDirectoryStructure(const std::string expFileName) {
		namespace bf = boost::filesystem;
		
		const std::string substitutionDescriptor("%s");
		
		std::string expName(removeFileExtension(expFileName));
		bf::path currExpStorageDir = getLocalExperimentStorageDir(expName);
		
		
		//bf::path expFileNamePath(expFileName);
		
		if(exists(currExpStorageDir)) {
			remove_all(currExpStorageDir);
		}
		
		bf::create_directories(currExpStorageDir);
		
		
	}
	
	
    void getFilePaths(const std::string &workingPath,
                      const std::string &directoryPath,
                      std::vector<std::string> &filePaths,
                      bool recursive)
    {
        const std::string fullPath(expandPath(workingPath, directoryPath).string());
        
        getFilePaths(fullPath, filePaths, recursive);

        if (fullPath != directoryPath) {
            for (std::vector<std::string>::iterator iter = filePaths.begin(); iter != filePaths.end(); iter++) {
                (*iter).erase(0, workingPath.size() + 1);  // +1 for the trailing forward-slash
            }
        }
    }


    static void getFilePathsInternal(const boost::filesystem::path &dirPath,
                                     std::vector<std::string> &filePaths,
                                     bool recursive)
    {
        namespace bf = boost::filesystem;
        
        bf::directory_iterator endIter;
        for (bf::directory_iterator iter(dirPath); iter != endIter; iter++) {
            // Include only regular files whose names don't start with "."
            const auto path = iter->path();
            if (path.filename().string().find(".") != 0) {
                if (bf::is_regular_file(iter->status())) {
                    filePaths.push_back(path.string());
                } else if (recursive && bf::is_directory(iter->status())) {
                    getFilePathsInternal(path, filePaths, true);
                }
            }
        }
    }

    
    void getFilePaths(const std::string &directoryPath, std::vector<std::string> &filePaths, bool recursive) {
        namespace bf = boost::filesystem;
        
        bf::path dirPath(directoryPath);
        if (!bf::is_directory(dirPath)) {
            throw SimpleException("Invalid directory path", directoryPath);
        }
        
        getFilePathsInternal(dirPath, filePaths, recursive);
        
        if (filePaths.size() == 0) {
            throw SimpleException("Directory contains no regular files", directoryPath);
        }
    }


std::size_t getMatchingFilenames(const std::string &workingPath,
                                 const string &globPattern,
                                 std::vector<std::string> &filenames)
{
    auto cwdfd = open(".", O_RDONLY);
    if (-1 == cwdfd) {
        throw SimpleException("Unable to open current working directory", strerror(errno));
    }
    BOOST_SCOPE_EXIT( &cwdfd ) {
        (void)fchdir(cwdfd);
        (void)close(cwdfd);
    } BOOST_SCOPE_EXIT_END
    
    if (!(workingPath.empty())) {
        if (-1 == chdir(workingPath.c_str())) {
            throw SimpleException("Unable to change directory", strerror(errno));
        }
    }
    
    glob_t globResults;
    auto globStatus = glob(globPattern.c_str(), 0, nullptr, &globResults);
    BOOST_SCOPE_EXIT( &globResults ) {
        globfree(&globResults);
    } BOOST_SCOPE_EXIT_END
    
    if ((0 != globStatus) && (GLOB_NOMATCH != globStatus)) {
        throw SimpleException("Unknown error when evaluating filenames pattern", globPattern);
    }
    
    for (std::size_t i = 0; i < globResults.gl_pathc; i++) {
        boost::filesystem::path filePath(globResults.gl_pathv[i]);
        filenames.push_back(filePath.string());
    }
    
    return globResults.gl_pathc;
}


END_NAMESPACE_MW






















