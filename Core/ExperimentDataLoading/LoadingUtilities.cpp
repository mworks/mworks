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
#include "SystemEventFactory.h"
#include "EventBuffer.h"
#include "StandardVariables.h"
#include "StandardStimuli.h"
#include "IODeviceManager.h"
#include "FilterManager.h"
#include "OpenALContextManager.h"
#include <string>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/pointer_cast.hpp>
#include "ComponentRegistry.h"
#include "XMLParser.h"
#include "DataFileManager.h"
using namespace mw;

namespace mw {

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
		
		
		shared_ptr<ComponentRegistry> reg = ComponentRegistry::getSharedRegistry();
		boost::filesystem::path setupPath(prependLocalPath("setup_variables.xml"));
		XMLParser parser(reg, setupPath.string());
		
		parser.validate();
		parser.parse();
		
		// check setup variables for validity
		shared_ptr<Variable> main_screen_info_variable = reg->getVariable(MAIN_SCREEN_INFO_TAGNAME);
		Datum main_screen_dict = main_screen_info_variable->getValue();
           

        if(!CHECK_DICT_VALUE_IS_NUMBER(main_screen_dict, M_DISPLAY_WIDTH_KEY) || 
           !CHECK_DICT_VALUE_IS_NUMBER(main_screen_dict, M_DISPLAY_HEIGHT_KEY) ||
           !CHECK_DICT_VALUE_IS_NUMBER(main_screen_dict, M_DISPLAY_DISTANCE_KEY) ||
           !CHECK_DICT_VALUE_IS_NUMBER(main_screen_dict, M_REFRESH_RATE_KEY)){
            
            throw SimpleException("Invalid display info defined in setup_variables.xml");
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
			merror(M_PARSER_MESSAGE_DOMAIN, e.what());
			GlobalCurrentExperiment = shared_ptr<Experiment>();
			global_outgoing_event_buffer->putEvent(SystemEventFactory::currentExperimentState());
			return false;
		}
		
// This is getting double loaded    
/*		try {
			loadSetupVariables();
		} catch(std::exception& e){
			merror(M_PARSER_MESSAGE_DOMAIN, "Failed to load setup variables.  Specific problems was: \"%s\"", e.what());
			GlobalCurrentExperiment = shared_ptr<Experiment>();
			global_outgoing_event_buffer->putEvent(SystemEventFactory::currentExperimentState());
			return false;
		}*/
		
		shared_ptr<ComponentRegistry> reg = ComponentRegistry::getSharedRegistry();
		
		try {
			XMLParser parser(reg, filepath.string());
			
			parser.validate();
			parser.parse(true);
			
		} catch(std::exception& e){
			merror(M_PARSER_MESSAGE_DOMAIN, e.what());
			GlobalCurrentExperiment = shared_ptr<Experiment>();
			global_outgoing_event_buffer->putEvent(SystemEventFactory::currentExperimentState());
			return false;
		}
		
		if(GlobalCurrentExperiment == NULL) {
			merror(M_PARSER_MESSAGE_DOMAIN, "Experiment load failed");
			global_outgoing_event_buffer->putEvent(SystemEventFactory::currentExperimentState());
			return false;
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
            opengl_context_manager->releaseDisplays();
            //opengl_context_manager->destroy();
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
			
			global_outgoing_event_buffer->putEvent(SystemEventFactory::componentCodecPackage());
			global_outgoing_event_buffer->putEvent(SystemEventFactory::codecPackage());
			
			global_outgoing_event_buffer->putEvent(SystemEventFactory::currentExperimentState());
			//global_outgoing_event_buffer->putEvent(SystemEventFactory::protocolPackage());
			global_variable_registry->announceAll();
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
        
        shared_ptr<OpenGLContextManager> opengl_context_manager = OpenGLContextManager::instance(false);
        if(opengl_context_manager == NULL){
            opengl_context_manager = shared_ptr<OpenGLContextManager>(new OpenGLContextManager()); 
            OpenGLContextManager::registerInstance(dynamic_pointer_cast<OpenGLContextManager, Component>(opengl_context_manager));
		}
        //opengl_context_manager->releaseDisplays();
		
        shared_ptr<ComponentRegistry> reg = ComponentRegistry::getSharedRegistry();
        shared_ptr<Variable> main_screen_info = reg->getVariable(MAIN_SCREEN_INFO_TAGNAME);
    
		bool always_display_mirror_window = 0;
		int display_to_use = 0;
		if(main_screen_info != NULL){
			
		 Datum val = *(main_screen_info);
			if(val.hasKey(M_DISPLAY_TO_USE_KEY)){
				display_to_use = (int)val.getElement(M_DISPLAY_TO_USE_KEY);
			}
			
			if(val.hasKey(M_ALWAYS_DISPLAY_MIRROR_WINDOW_KEY)){
				always_display_mirror_window = (bool)val.getElement(M_ALWAYS_DISPLAY_MIRROR_WINDOW_KEY);
			}
		}
		
		shared_ptr<StimulusDisplay> stimdisplay(new StimulusDisplay());
		int new_context = -1;
        
        
		if(display_to_use >= 0 && (opengl_context_manager->getNMonitors() > 1 || display_to_use == 0)) {
			
			if(display_to_use >= opengl_context_manager->getNMonitors()) {
				merror(M_SERVER_MESSAGE_DOMAIN,
					   "Requested display index (%d) is greater than the number of displays (%d)", 
					   display_to_use, 
					   opengl_context_manager->getNMonitors());
				merror(M_SERVER_MESSAGE_DOMAIN, "Using default display");
				display_to_use = 1;						   			
			}
			
			new_context = opengl_context_manager->newFullscreenContext(32,display_to_use);
			stimdisplay->addContext(new_context);
			
			if(always_display_mirror_window){
				int auxilliary_context = opengl_context_manager->newMirrorContext(32); 
				stimdisplay->addContext(auxilliary_context);
			}
			
			opengl_context_manager->setMainDisplayIndex(display_to_use);
		} else {
			opengl_context_manager->setMainDisplayIndex(0);
			new_context = opengl_context_manager->newMirrorContext(32);
			stimdisplay->addContext(new_context);		
		}
		
		
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

