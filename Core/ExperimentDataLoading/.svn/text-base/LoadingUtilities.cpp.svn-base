/**
 * LoadingUtilities.cpp
 * 
 * Copyright (c) 2004 MIT. All rights reserved.
 */

// Platform dependence TODO
#include "OpenGLContextManager.h"

#include "Event.h"
#include "LoadingUtilities.h"
#include "Experiment.h"
#include "EmbeddedPerlInterpreter.h"
#include "PlatformDependentServices.h"
#include "EventFactory.h"
#include "EventBuffer.h"
#include "StandardVariables.h"
#include "StandardStimuli.h"
#include "IODeviceManager.h"
#include "FilterManager.h"
#include "OpenALContextManager.h"
#include <string>
#include "boost/filesystem/path.hpp"
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/convenience.hpp"
#include "ComponentRegistry_new.h"
#include "XMLParser.h"
#include "DataFileManager.h"
using namespace mw;

namespace mw {
	Data checkLoadedValueAndReturnNewValuesWithLoadedValueOrDefaultIfNotNumber(const Data main_window_values, 
																			   const Data current_values, 
																			   const std::string &key, 
																			   const Data default_values) {
		Data new_values(current_values);
		if(!(main_window_values.getElement(key).isNumber())) {
			merror(M_PARSER_MESSAGE_DOMAIN, "'%s' is either empty or has an illegal value, setting default to %f", key.c_str(), default_values.getFloat());
			new_values.addElement(key.c_str(), default_values);
		} else {
			new_values.addElement(key.c_str(), main_window_values.getElement(key));
		}
		
		return new_values;
	}
	
	
	bool loadSetupVariables() {
		
		
		shared_ptr<mwComponentRegistry> reg = mwComponentRegistry::getSharedRegistry();
		boost::filesystem::path setupPath(prependLocalPath("setup_variables.xml"));
		XMLParser parser(reg, setupPath.string());
		
		parser.validate();
		parser.parse();
		
		// check setup variables for validity
		shared_ptr<Variable> main_screen_info = reg->getVariable(MAIN_SCREEN_INFO_TAGNAME);
		
		Data loaded_values = main_screen_info->getValue();
		Data new_values(M_DICTIONARY, 7);
		
		new_values = checkLoadedValueAndReturnNewValuesWithLoadedValueOrDefaultIfNotNumber(loaded_values, new_values, M_DISPLAY_WIDTH_KEY, 19.0);
		new_values = checkLoadedValueAndReturnNewValuesWithLoadedValueOrDefaultIfNotNumber(loaded_values, new_values, M_DISPLAY_HEIGHT_KEY, 11.875);
		new_values = checkLoadedValueAndReturnNewValuesWithLoadedValueOrDefaultIfNotNumber(loaded_values, new_values, M_DISPLAY_DISTANCE_KEY, 20.0);
		new_values = checkLoadedValueAndReturnNewValuesWithLoadedValueOrDefaultIfNotNumber(loaded_values, new_values, M_REFRESH_RATE_KEY, 60L);
		
		if(!(loaded_values.getElement(M_ALWAYS_DISPLAY_MIRROR_WINDOW_KEY).isNumber())) {
			merror(M_PARSER_MESSAGE_DOMAIN, "'always_display_mirror_window' is either empty or has an illegal value, setting default to 0");
			new_values.addElement(M_ALWAYS_DISPLAY_MIRROR_WINDOW_KEY, 0L);
		} else {
			long always_display_mirror_window = loaded_values.getElement(M_ALWAYS_DISPLAY_MIRROR_WINDOW_KEY).getInteger();
			if(!(always_display_mirror_window != 0 || always_display_mirror_window != 1)) {
				merror(M_PARSER_MESSAGE_DOMAIN, "'always_display_mirror_window' has an illegal value (%d), setting default to 0", always_display_mirror_window);
				new_values.addElement(M_ALWAYS_DISPLAY_MIRROR_WINDOW_KEY, 0L);				
			} else {
				new_values.addElement(M_ALWAYS_DISPLAY_MIRROR_WINDOW_KEY, always_display_mirror_window);								
				
				if(always_display_mirror_window == 1) {
					// now check the base height
					new_values = checkLoadedValueAndReturnNewValuesWithLoadedValueOrDefaultIfNotNumber(loaded_values, new_values, M_MIRROR_WINDOW_BASE_HEIGHT_KEY, 400L);
				}
			}
		}		
		
		if(!(loaded_values.getElement(M_DISPLAY_TO_USE_KEY).isInteger())) {
			merror(M_PARSER_MESSAGE_DOMAIN, "'%s' is either empty or has an illegal value, setting default to 0", M_DISPLAY_TO_USE_KEY);
			new_values.addElement(M_DISPLAY_TO_USE_KEY, 0L);
		} else {
			new_values.addElement(M_DISPLAY_TO_USE_KEY, loaded_values.getElement(M_DISPLAY_TO_USE_KEY));								
		}
		
		main_screen_info->setValue(new_values);
		return true;
	}
	
	bool loadExperimentFromXMLParser(const boost::filesystem::path filepath) {
		const std::string substitutionDescriptor("%s");
		
		
		try {
			unloadExperiment(false); // delete an experiment if one exists.
		} catch(std::exception& e){
			merror(M_PARSER_MESSAGE_DOMAIN, e.what());
			GlobalCurrentExperiment = shared_ptr<Experiment>();
			GlobalBufferManager->putEvent(EventFactory::currentExperimentState());
			return false;
		}
		
		
		try {
			loadSetupVariables();
		} catch(std::exception& e){
			merror(M_PARSER_MESSAGE_DOMAIN, "Failed to load setup variables.  Specific problems was: \"%s\"", e.what());
			GlobalCurrentExperiment = shared_ptr<Experiment>();
			GlobalBufferManager->putEvent(EventFactory::currentExperimentState());
			return false;
		}
		
		shared_ptr<mwComponentRegistry> reg = mwComponentRegistry::getSharedRegistry();
		
		try {
			XMLParser parser(reg, filepath.string());
			
			parser.validate();
			parser.parse(true);
			
		} catch(std::exception& e){
			merror(M_PARSER_MESSAGE_DOMAIN, e.what());
			GlobalCurrentExperiment = shared_ptr<Experiment>();
			GlobalBufferManager->putEvent(EventFactory::currentExperimentState());
			return false;
		}
		
		if(GlobalCurrentExperiment == NULL) {
			merror(M_PARSER_MESSAGE_DOMAIN, "Experiment load failed");
			GlobalBufferManager->putEvent(EventFactory::currentExperimentState());
			return false;
		}
		
		GlobalBufferManager->putEvent(EventFactory::componentCodecPackage());
		GlobalBufferManager->putEvent(EventFactory::codecPackage());
		GlobalBufferManager->putEvent(EventFactory::currentExperimentState());
		GlobalBufferManager->putEvent(EventFactory::protocolPackage());
		GlobalVariableRegistry->announceAll();
		
		return true;
	}
	
	void parseIncludedFilesFromExperiment(const boost::filesystem::path filepath) {
		// DEPRECATED
	}
	
	
	void unloadExperiment(bool announce) {
		
		GlobalCurrentExperiment = shared_ptr<Experiment>();
		
		shared_ptr<mwComponentRegistry> component_registry = mwComponentRegistry::getSharedRegistry();
		component_registry->resetInstances();
		
		if(GlobalVariableRegistry != NULL) {	// exp. already loaded
			GlobalVariableRegistry = shared_ptr<VariableRegistry>(new VariableRegistry(GlobalBufferManager));
			initializeStandardVariables(GlobalVariableRegistry);
			loadSetupVariables();
		}
		
		if(GlobalOpenGLContextManager != NULL){
			GlobalOpenGLContextManager->releaseDisplays();
		}
		OpenGLImageLoader::initialized = false;
		
		
		
		// kill all IODevices
		if(GlobalIODeviceManager != NULL) {
			delete GlobalIODeviceManager;  
		}
		
		
		// kill all filters
		if(GlobalFilterManager != NULL) {
			delete GlobalFilterManager;  
		}
		
		// these may not be needed here because they are also in the parser.
		GlobalIODeviceManager = new IODeviceManager();
		
		GlobalFilterManager = new FilterManager();
		
		if(GlobalOpenALContextManager != NULL){
			delete GlobalOpenALContextManager;
		}
		
		GlobalOpenALContextManager = new OpenALContextManager();
		
		if(GlobalDataFileManager != NULL && GlobalDataFileManager->isFileOpen()){
			GlobalDataFileManager->closeFile();
		}
		
		if(announce){
			
			GlobalBufferManager->putEvent(EventFactory::componentCodecPackage());
			GlobalBufferManager->putEvent(EventFactory::codecPackage());
			
			GlobalBufferManager->putEvent(EventFactory::currentExperimentState());
			//GlobalBufferManager->putEvent(EventFactory::protocolPackage());
			GlobalVariableRegistry->announceAll();
		}
	}
	
	boost::filesystem::path expandPath(std::string working_directory, std::string path){
		
		boost::filesystem::path specific_path;
		boost::filesystem::path full_path;
		
		try {
			specific_path = boost::filesystem::path(path, boost::filesystem::native);
		} catch(std::exception& e){
			cerr << "bad path" << endl;
		}
		
		if(path[0] == '/'){
			return specific_path;
		} else {
			try{
				
				boost::filesystem::path working_path(working_directory, boost::filesystem::native);
				full_path = working_path / specific_path;
			} catch(std::exception& e){
				cerr << "bad path" << endl;
			}
			
			return full_path;
		}
		
	}
	
	
	void prepareStimulusDisplay() {
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
		//	mprintf("Preparing stimulus display");
		if(GlobalOpenGLContextManager == NULL) {
			GlobalOpenGLContextManager = new OpenGLContextManager();
		}
		
		GlobalOpenGLContextManager->releaseDisplays();
		
		bool always_display_mirror_window = 0;
		int display_to_use = 1;
		if(mainDisplayInfo != NULL){
			
			Data val = *(mainDisplayInfo);
			if(val.hasKey(M_DISPLAY_TO_USE_KEY)){
				display_to_use = (int)val.getElement(M_DISPLAY_TO_USE_KEY);
			}
			
			if(val.hasKey(M_ALWAYS_DISPLAY_MIRROR_WINDOW_KEY)){
				always_display_mirror_window = (bool)val.getElement(M_ALWAYS_DISPLAY_MIRROR_WINDOW_KEY);
			}
		}
		
		shared_ptr<StimulusDisplay> stimdisplay(new StimulusDisplay());
		int new_context = -1;
		if(GlobalOpenGLContextManager->systemHasSecondMonitor() || display_to_use == 0) {
			
			if(display_to_use >= GlobalOpenGLContextManager->getNMonitors()) {
				merror(M_SERVER_MESSAGE_DOMAIN,
					   "Requested display index (%d) is greater than the number of displays (%d)", 
					   display_to_use, 
					   GlobalOpenGLContextManager->getNMonitors());
				merror(M_SERVER_MESSAGE_DOMAIN, "Using default display");
				display_to_use = 1;						   			
			}
			
			new_context = GlobalOpenGLContextManager->newFullScreenContext(32,display_to_use);
			stimdisplay->addContext(new_context);
			
			if(always_display_mirror_window){
				int auxilliary_context = GlobalOpenGLContextManager->newMirrorContext(32); 
				stimdisplay->addContext(auxilliary_context);
			}
			
			GlobalOpenGLContextManager->setMainDisplayIndex(display_to_use);
		} else {
			GlobalOpenGLContextManager->setMainDisplayIndex(0);
			new_context = GlobalOpenGLContextManager->newMirrorContext(32);
			stimdisplay->addContext(new_context);		
		}
		
		
#ifdef USE_COCOA_OPENGL
		//[GlobalStimMirrorController hide];
#endif
		
		
		/*    if(!stimdisplay) {
		 mwarning(M_DISPLAY_MESSAGE_DOMAIN, "Failed to create a valid stimulus display.");
		 mwarning(M_DISPLAY_MESSAGE_DOMAIN, "Attempting to switch to mirror");
		 stimdisplay->addContext(GlobalOpenGLContextManager->newMirrorContext(28));
		 if(!stimdisplay){ // if still no good...
		 merror(M_DISPLAY_MESSAGE_DOMAIN, "Failed to create stimulus display!");
		 merror(M_DISPLAY_MESSAGE_DOMAIN, "Unable to continue");
		 return;
		 }
		 }*/
		GlobalCurrentExperiment->setStimulusDisplay(stimdisplay);
		stimdisplay->updateDisplay();
		[pool release];
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
		bf::path temp(experimentInstallPath());
		bf::path currExpStorageDir = temp / 
		bf::path(expName, bf::native) / 
		experimentStorageDirectoryName();
		
		bf::path currExpParentDir = temp / 
		bf::path(expName, bf::native);
		
		//bf::path expFileNamePath(expFileName);
		
		if(exists(currExpStorageDir)) {
			remove_all(currExpStorageDir);
		}
		
		bf::create_directory(currExpParentDir);
		bf::create_directory(currExpStorageDir);
		
		
	}
	
	
	std::string removeSpacesFromString(std::string s) {
		const std::string us("_");
		const std::string space(" ");
		
		while(s.find_first_of(space) != std::string::npos) {
			s.replace(s.find_first_of(space), us.length(), us);
		}
		return s;
	}
}

