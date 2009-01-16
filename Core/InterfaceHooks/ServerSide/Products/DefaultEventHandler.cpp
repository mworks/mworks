/**
* DefaultEventHandler.cpp
 *
 * History:
 * paul on 1/27/06 - Created.
 *
 * Copyright 2006 MIT. All rights reserved.
 */

#include "DefaultEventHandler.h"
#include "EventFactory.h"
#include "VariableSave.h"
#include "VariableLoad.h"
#include "Utilities.h"
#include "ExperimentUnpackager.h"
#include "LoadingUtilities.h"
#include "PlatformDependentServices.h"
#include "EventBuffer.h"
#include "Experiment.h"
#include "DataFileManager.h"
#include "StandardInterpreters.h"
#include "GenericData.h"
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include "StateSystem.h"
using namespace mw;


void DefaultEventHandler::handleEvent(shared_ptr <Event> &evt) {
	shared_ptr<Variable> var(GlobalVariableRegistry->getVariable(evt->getEventCode()));
	
	if(var != 0) {		
		try {
			var->setValue(evt->getData());		 
			
			if (GlobalSystemEventVariable->getCodecCode() == evt->getEventCode()) {
				handleSystemEvent(evt->getData());
			}
		} catch (void *e) {
			merror(M_NETWORK_MESSAGE_DOMAIN, "Server received illegal event code: %d", evt->getEventCode());
		}
	}
}

void DefaultEventHandler::handleSystemEvent(const Data &sysEvent) {
	Data payloadType = sysEvent.getElement(M_SYSTEM_PAYLOAD_TYPE);
	if(!payloadType.isInteger()) {
		merror(M_NETWORK_MESSAGE_DOMAIN, 
			   "badly formed system event");
		return;
	}
	
    switch((SystemPayloadType)payloadType.getInteger()) {
        case M_EXPERIMENT_PACKAGE:
		{
			namespace bf = boost::filesystem;
			Data payload(sysEvent.getElement(M_SYSTEM_PAYLOAD));
			
			
            ExperimentUnpackager unpackage;
            if(!unpackage.unpackageExperiment(payload)) {
                merror(M_NETWORK_MESSAGE_DOMAIN, 
					   "Unable to unpackage experiment");
                // TODO: throw, or at least, issue an event indicating that the
				// load failed
				break;
            }
            
			bf::path expXMLFile(unpackage.getUnpackagedExperimentPath());
			
			
			if(!loadExperimentFromXMLParser(expXMLFile)) {
                merror(M_PARSER_MESSAGE_DOMAIN, 
					   "Failed to parse experiment %s", 
					   expXMLFile.string().c_str());
				// TODO: throw, or at least, issue an event indicating that the
				// load failed
                break;
            }
			
			cerr << expXMLFile.string() << endl;
            
			bf::path expPath(expXMLFile.branch_path().branch_path());
			
            cerr << expPath.string() << endl;
            
			GlobalCurrentExperiment->setExperimentPath(expPath.string().c_str());
			shared_ptr<Event> experimentStateEvent = EventFactory::currentExperimentState();
			GlobalBufferManager->putEvent(experimentStateEvent);
			mprintf("%s successfully loaded, using protocol: %s", 
					GlobalCurrentExperiment->getExperimentName().c_str(),
					GlobalCurrentExperiment->getCurrentProtocol()->getName().c_str());
			
            break;
		}  
        case M_PROTOCOL_SELECTION:
		{
			Data payload(sysEvent.getElement(M_SYSTEM_PAYLOAD));
			
			std::string newProtocol(payload.getString());
            if(newProtocol.size() == 0) {
                merror(M_PARADIGM_MESSAGE_DOMAIN, 
					   "Selected protocol is NULL");
                break;
            }
			
            GlobalCurrentExperiment->setCurrentProtocol(newProtocol);
			break;
		}            
        case M_CLOSE_EXPERIMENT:
		{
			Data payload(sysEvent.getElement(M_SYSTEM_PAYLOAD));
			
			std::string tmp(GlobalCurrentExperiment->getExperimentName());
			std::string expName(payload.getString());
            if(expName.size() == 0) {
                merror(M_SYSTEM_MESSAGE_DOMAIN, 
					   "Tried to close NULL experiment");
                break;
            }
            // make sure the experiment we are trying to close is the one
            // currently loaded.
           /* if(tmp != expName){
                mwarning(M_SYSTEM_MESSAGE_DOMAIN,
						 "Attempted to close an experiment not in context\n"
						 "(name: %s) (current: %s)", 
						 expName.c_str(), tmp.c_str());
                break;
            }*/
			
            // release all the memory
            unloadExperiment();
            // we no longer need this
			break;
		}
		case M_START_EXPERIMENT:
		{
			shared_ptr <StateSystem> state_system = StateSystem::instance();
			state_system->start();
		}
			break;
			
		case M_STOP_EXPERIMENT:
		{
			shared_ptr <StateSystem> state_system = StateSystem::instance();
			state_system->stop();
		}
			break;
			
		case M_OPEN_DATA_FILE:
		{
			// issues an event on success
			GlobalDataFileManager->openFile(sysEvent.getElement(M_SYSTEM_PAYLOAD));
			break;
		}
		case M_CLOSE_DATA_FILE:
			GlobalDataFileManager->closeFile();
			break;
		case M_SAVE_VARIABLES:
		{
			namespace bf= boost::filesystem;
			
			Data payload(sysEvent.getElement(M_SYSTEM_PAYLOAD));
			
			Data fullPath(payload.getElement(SAVE_VARIABLES_FULL_PATH));
			Data file(payload.getElement(SAVE_VARIABLES_FILE));
			Data overwrite(payload.getElement(SAVE_VARIABLES_OVERWRITE));
			
			if(!fullPath.isInteger() || !file.isString() || !overwrite.isInteger()) {
				merror(M_STATE_SYSTEM_MESSAGE_DOMAIN,
						 "illegal property in save variables event");				
			} else {

				bf::path filename;
				// make sure the proper directory structure exists
                
                string filestring;
                
                try {
                    filestring = file.getString();
                } catch(std::exception& e){
                    merror(M_SYSTEM_MESSAGE_DOMAIN, "Invalid filename for variable set");
                }
                
				try {
                
                    bf::path experimentPath = bf::path(GlobalCurrentExperiment->getExperimentPath(), bf::native);
                    bf::path variablesDirectory = experimentPath / SAVED_VARIABLES_DIR_NAME;
                    
                    if(bf::create_directory(variablesDirectory)) {
                        mprintf("Creating saved variables directory");
                    }
                    
                    if(fullPath.getBool()) {
                        filename = bf::path(file.getString(), boost::filesystem::native);				
                    } else {
                        filename = variablesDirectory / bf::path(appendFileExtension(filestring, ".xml"), bf::native);
                    }

                } catch (std::exception& e){
                    merror(M_SYSTEM_MESSAGE_DOMAIN, "Error interacting with file system to save variables: %s", e.what());
                }
                
				if((bf::exists(filename) && overwrite.getBool()) || !bf::exists(filename)) {
					try {
                        VariableSave::saveExperimentwideVariables(filename);
                    } catch (std::exception& e){
                        merror(M_SYSTEM_MESSAGE_DOMAIN, "An error occurred while saving variables");
                    }
				} else {
					mwarning(M_STATE_SYSTEM_MESSAGE_DOMAIN,
							 "NOT overwriting current variables");
				}
			}			

			shared_ptr<Event> experimentStateEvent = EventFactory::currentExperimentState();
			GlobalBufferManager->putEvent(experimentStateEvent);

			break;
		}	
		case M_LOAD_VARIABLES:
		{
			namespace bf= boost::filesystem;
			Data payload(sysEvent.getElement(M_SYSTEM_PAYLOAD));
			
			Data fullPath(payload.getElement(LOAD_VARIABLES_FULL_PATH));
			Data file(payload.getElement(LOAD_VARIABLES_FILE));
			
			if(!fullPath.isInteger() || !file.isString()) {
				merror(M_STATE_SYSTEM_MESSAGE_DOMAIN,
					   "illegal property in load variables event");				
			}
			
			bf::path filename;
			if(fullPath.getBool()) {
				filename = bf::path(file.getString(), boost::filesystem::native);				
			} else {
				filename = bf::path(GlobalCurrentExperiment->getExperimentPath(), bf::native) / 
				SAVED_VARIABLES_DIR_NAME /
				bf::path(appendFileExtension(file.getString(), ".xml"), bf::native);
			}
			
			VariableLoad::loadExperimentwideVariables(filename);
			
			break;
		}	
		default:
			break;
	}
}
