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
#include "OpenALContextManager.h"
#include <string>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/pointer_cast.hpp>
#include "ComponentRegistry.h"
#include "XMLParser.h"
#include "DataFileManager.h"
#include <glob.h>
#include <boost/scope_exit.hpp>


BEGIN_NAMESPACE_MW


// DDC: HOLY CRAP: WHY WAS HE QUIETLY REPLACING MISSING VALUES WITH DEFAULTS?!?
// Datum checkLoadedValueAndReturnNewValuesWithLoadedValueOrDefaultIfNotNumber(const Datum main_window_values, 
//																			   const Datum current_values, 
//																			   const std::string &key, 
//																			   const Datum default_values) {
//	 Datum new_values(current_values);
//		if(!(main_window_values.getElement(key).isNumber())) {
//			merror(M_PARSER_MESSAGE_DOMAIN, "'%s' is either empty or has an illegal value, setting default to %f", key.c_str(), default_values.getFloat());
//			new_values.addElement(key.c_str(), default_values);
//		} else {
//			new_values.addElement(key.c_str(), main_window_values.getElement(key));
//		}
//		
//		return new_values;
//	}
	
#define CHECK_DICT_VALUE_IS_NUMBER(dict, key) dict.getElement(key).isNumber() 
    
	bool loadSetupVariables() {
		
		
		boost::filesystem::path setupPath(setupVariablesFile());
		shared_ptr<ComponentRegistry> reg = ComponentRegistry::getSharedRegistry();
		XMLParser parser(reg, setupPath.string());
		
		parser.parse();
		
		// check setup variables for validity
		shared_ptr<Variable> main_screen_info_variable = reg->getVariable(MAIN_SCREEN_INFO_TAGNAME);
		Datum main_screen_dict = main_screen_info_variable->getValue();
           
        bool width_ok = CHECK_DICT_VALUE_IS_NUMBER(main_screen_dict, M_DISPLAY_WIDTH_KEY);
        bool height_ok = CHECK_DICT_VALUE_IS_NUMBER(main_screen_dict, M_DISPLAY_HEIGHT_KEY);
        bool dist_ok = CHECK_DICT_VALUE_IS_NUMBER(main_screen_dict, M_DISPLAY_DISTANCE_KEY);
        bool refresh_ok = CHECK_DICT_VALUE_IS_NUMBER(main_screen_dict, M_REFRESH_RATE_KEY);
        
        if (!(width_ok && height_ok && dist_ok && refresh_ok)) {
            string stem = "Invalid display info defined in setup_variables.xml (missing/invalid: ";
            if(!width_ok) stem = stem + M_DISPLAY_WIDTH_KEY + " ";
            if(!height_ok) stem = stem + M_DISPLAY_HEIGHT_KEY + " ";
            if(!dist_ok) stem = stem + M_DISPLAY_DISTANCE_KEY + " ";
            if(!refresh_ok) stem = stem + M_REFRESH_RATE_KEY + " ";
            stem += ")";
            throw SimpleException(stem);
        }
        
        // TODO: more error checking.  THESE SHOULD ALL THROW IF SOMETHING IS WRONG!!!
        
        
        
        // DDC: HOLY CRAP!  Why was he quietly replacing missing values with defaults?  This is a recipe for 
        //      serious errors quietly creeping in!
        
        //Datum loaded_values = main_screen_info->getValue();
//        Datum new_values(M_DICTIONARY, 7);
//		
//		new_values = checkLoadedValueAndReturnNewValuesWithLoadedValueOrDefaultIfNotNumber(loaded_values, new_values, M_DISPLAY_WIDTH_KEY, 14.75);
//		new_values = checkLoadedValueAndReturnNewValuesWithLoadedValueOrDefaultIfNotNumber(loaded_values, new_values, M_DISPLAY_HEIGHT_KEY, 11.09);
//		new_values = checkLoadedValueAndReturnNewValuesWithLoadedValueOrDefaultIfNotNumber(loaded_values, new_values, M_DISPLAY_DISTANCE_KEY, 10.0);
//		new_values = checkLoadedValueAndReturnNewValuesWithLoadedValueOrDefaultIfNotNumber(loaded_values, new_values, M_REFRESH_RATE_KEY, 60L);
//		
//		if(!(loaded_values.getElement(M_ALWAYS_DISPLAY_MIRROR_WINDOW_KEY).isNumber())) {
//			merror(M_PARSER_MESSAGE_DOMAIN, "'always_display_mirror_window' is either empty or has an illegal value, setting default to 0");
//			new_values.addElement(M_ALWAYS_DISPLAY_MIRROR_WINDOW_KEY, 0L);
//		} else {
//			long always_display_mirror_window = loaded_values.getElement(M_ALWAYS_DISPLAY_MIRROR_WINDOW_KEY).getInteger();
//			if(!(always_display_mirror_window != 0 || always_display_mirror_window != 1)) {
//				merror(M_PARSER_MESSAGE_DOMAIN, "'always_display_mirror_window' has an illegal value (%d), setting default to 0", always_display_mirror_window);
//				new_values.addElement(M_ALWAYS_DISPLAY_MIRROR_WINDOW_KEY, 0L);				
//			} else {
//				new_values.addElement(M_ALWAYS_DISPLAY_MIRROR_WINDOW_KEY, always_display_mirror_window);								
//				
//				if(always_display_mirror_window == 1) {
//					// now check the base height
//					new_values = checkLoadedValueAndReturnNewValuesWithLoadedValueOrDefaultIfNotNumber(loaded_values, new_values, M_MIRROR_WINDOW_BASE_HEIGHT_KEY, 400L);
//				}
//			}
//		}		
//		
//		if(!(loaded_values.getElement(M_DISPLAY_TO_USE_KEY).isInteger())) {
//			merror(M_PARSER_MESSAGE_DOMAIN, "'%s' is either empty or has an illegal value, setting default to 0", M_DISPLAY_TO_USE_KEY);
//			new_values.addElement(M_DISPLAY_TO_USE_KEY, 0L);
//		} else {
//			new_values.addElement(M_DISPLAY_TO_USE_KEY, loaded_values.getElement(M_DISPLAY_TO_USE_KEY));								
//		}
//		
//		main_screen_info->setValue(new_values);
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
		
		if(GlobalCurrentExperiment == NULL) {
			merror(M_PARSER_MESSAGE_DOMAIN, "Experiment load failed");
			global_outgoing_event_buffer->putEvent(SystemEventFactory::currentExperimentState());
			return false;
		}
        
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
	
	void parseIncludedFilesFromExperiment(const boost::filesystem::path filepath) {
		// DEPRECATED
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
		
        OpenALContextManager::destroy();
        shared_ptr<Component> openal_context_manager(new OpenALContextManager());
        OpenALContextManager::registerInstance(openal_context_manager);
        
		
		if(GlobalDataFileManager != NULL && GlobalDataFileManager->isFileOpen()){
			GlobalDataFileManager->closeFile();
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
	
	
	void prepareStimulusDisplay() {
		//	mprintf("Preparing stimulus display");
        
        shared_ptr<OpenGLContextManager> opengl_context_manager = OpenGLContextManager::instance(false);
        if(opengl_context_manager == NULL){
            opengl_context_manager = OpenGLContextManager::createPlatformOpenGLContextManager();
            OpenGLContextManager::registerInstance(opengl_context_manager);
		}
        //opengl_context_manager->releaseContexts();
		
        shared_ptr<ComponentRegistry> reg = ComponentRegistry::getSharedRegistry();
        shared_ptr<Variable> main_screen_info = reg->getVariable(MAIN_SCREEN_INFO_TAGNAME);
    
		bool always_display_mirror_window = 0;
		int display_to_use = 0;
        bool announce_individual_stimuli = true;
        bool render_at_full_resolution = true;
        bool use_color_management = true;
        
		if(main_screen_info != NULL){
			
		 Datum val = *(main_screen_info);
			if(val.hasKey(M_DISPLAY_TO_USE_KEY)){
				display_to_use = (int)val.getElement(M_DISPLAY_TO_USE_KEY);
			}
			
			if(val.hasKey(M_ALWAYS_DISPLAY_MIRROR_WINDOW_KEY)){
				always_display_mirror_window = (bool)val.getElement(M_ALWAYS_DISPLAY_MIRROR_WINDOW_KEY);
			}
			
			if(val.hasKey(M_ANNOUNCE_INDIVIDUAL_STIMULI_KEY)){
				announce_individual_stimuli = (bool)val.getElement(M_ANNOUNCE_INDIVIDUAL_STIMULI_KEY);
			}
            
            if(val.hasKey(M_RENDER_AT_FULL_RESOLUTION)){
                render_at_full_resolution = (bool)val.getElement(M_RENDER_AT_FULL_RESOLUTION);
            }
            
            if(val.hasKey(M_USE_COLOR_MANAGEMENT)){
                use_color_management = (bool)val.getElement(M_USE_COLOR_MANAGEMENT);
            }
		}
		
        auto stimdisplay = StimulusDisplay::createPlatformStimulusDisplay(announce_individual_stimuli,
                                                                          use_color_management);
		int new_context = -1;
        
        
		if(display_to_use >= 0 && (opengl_context_manager->getNumDisplays() > 1 || display_to_use == 0)) {
			
			if(display_to_use >= opengl_context_manager->getNumDisplays()) {
				merror(M_SERVER_MESSAGE_DOMAIN,
					   "Requested display index (%d) is greater than the number of displays (%d)", 
					   display_to_use, 
					   opengl_context_manager->getNumDisplays());
				merror(M_SERVER_MESSAGE_DOMAIN, "Using default display");
				display_to_use = 1;						   			
			}
			
			new_context = opengl_context_manager->newFullscreenContext(display_to_use, render_at_full_resolution);
			stimdisplay->addContext(new_context);
			
			if(always_display_mirror_window){
				int auxilliary_context = opengl_context_manager->newMirrorContext(render_at_full_resolution);
				stimdisplay->addContext(auxilliary_context);
			}
			
		} else {
			new_context = opengl_context_manager->newMirrorContext(render_at_full_resolution);
			stimdisplay->addContext(new_context);		
		}
		
		
		GlobalCurrentExperiment->setStimulusDisplay(stimdisplay);
		stimdisplay->clearDisplay();
	}
	
	
	void modifyExperimentMediaPaths(const boost::filesystem::path filepath) {
		const std::string substitutionDescriptor("%s");
		
	}
	
	void expandRangeReplicatorItems(const boost::filesystem::path filepath) {
		
		// TODO: deprecated
		
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
	
	
	std::string removeSpacesFromString(std::string s) {
		const std::string us("_");
		const std::string space(" ");
		
		while(s.find_first_of(space) != std::string::npos) {
			s.replace(s.find_first_of(space), us.length(), us);
		}
		return s;
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






















