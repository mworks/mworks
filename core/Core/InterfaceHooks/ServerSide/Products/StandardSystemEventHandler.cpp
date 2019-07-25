/**
* StandardSystemEventHandler.cpp
 *
 * History:
 * paul on 1/27/06 - Created.
 *
 * Copyright 2006 MIT. All rights reserved.
 */

#include "Experiment.h"
#include "StandardSystemEventHandler.h"
#include "SystemEventFactory.h"
#include "VariableSave.h"
#include "VariableLoad.h"
#include "Utilities.h"
#include "ExperimentUnpackager.h"
#include "LoadingUtilities.h"
#include "PlatformDependentServices.h"
#include "EventBuffer.h"
#include "DataFileManager.h"
#include "GenericData.h"
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include "StateSystem.h"


BEGIN_NAMESPACE_MW


StandardSystemEventHandler::StandardSystemEventHandler() : EventStreamInterface(M_SERVER_MESSAGE_DOMAIN) { }

void StandardSystemEventHandler::handleEvent(shared_ptr <Event> evt) {
    
    int event_code = evt->getEventCode();
    
    
    if(event_code == RESERVED_SYSTEM_EVENT_CODE){
        handleSystemEvent(evt->getData());
        return;
    }
    
    if(event_code < N_RESERVED_CODEC_CODES){
        // a kind of event to which we won't respond
        return;
    }
    
    if(event_code > N_RESERVED_CODEC_CODES + global_variable_registry->getNVariables()){
        // an invalid event
        mwarning(M_NETWORK_MESSAGE_DOMAIN, "Server received illegal event code: %d", evt->getEventCode());
        return;
    }
    
    try {
        shared_ptr<Variable> var(global_variable_registry->getVariable(evt->getEventCode()));
    
        var->setValue(evt->getData());		 
        
    } catch (void *e) {
        merror(M_NETWORK_MESSAGE_DOMAIN, "Server received illegal event code: %d", evt->getEventCode());
    }
	
}

void StandardSystemEventHandler::handleSystemEvent(const Datum &sysEvent) {
 Datum payloadType = sysEvent.getElement(M_SYSTEM_PAYLOAD_TYPE);
	if(!payloadType.isInteger()) {
		merror(M_NETWORK_MESSAGE_DOMAIN, 
			   "badly formed system event");
		return;
	}
	
    switch((SystemPayloadType)payloadType.getInteger()) {
        case M_EXPERIMENT_PACKAGE:
		{
			namespace bf = boost::filesystem;
		 Datum payload(sysEvent.getElement(M_SYSTEM_PAYLOAD));
			
			
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
				
                break;
            }
			
			//cerr << expXMLFile.string() << endl;
            
			bf::path expPath(expXMLFile.branch_path().branch_path());
			
            //cerr << expPath.string() << endl;
            
			GlobalCurrentExperiment->setExperimentPath(expPath.string().c_str());
			shared_ptr<Event> experimentStateEvent = SystemEventFactory::currentExperimentState();
			global_outgoing_event_buffer->putEvent(experimentStateEvent);
			mprintf("%s successfully loaded, using protocol: %s", 
					GlobalCurrentExperiment->getExperimentName().c_str(),
					GlobalCurrentExperiment->getCurrentProtocol()->getName().c_str());
			
            break;
		}  
        case M_PROTOCOL_SELECTION:
		{
		 Datum payload(sysEvent.getElement(M_SYSTEM_PAYLOAD));
			
			std::string newProtocol(payload.getString());
            if(newProtocol.size() == 0) {
                merror(M_PARADIGM_MESSAGE_DOMAIN, 
					   "Selected protocol is NULL");
                break;
            }
			
            GlobalCurrentExperiment->setCurrentProtocol(newProtocol);
			break;
		}
        case M_REQUEST_CODEC:
        {
            global_outgoing_event_buffer->putEvent(SystemEventFactory::codecPackage());
            break;
        }
        case M_REQUEST_COMPONENT_CODEC:
        {
            global_outgoing_event_buffer->putEvent(SystemEventFactory::componentCodecPackage());
            break;
        }
        case M_REQUEST_EXPERIMENT_STATE:
        {
            global_outgoing_event_buffer->putEvent(SystemEventFactory::currentExperimentState());
            break;
        }
        case M_REQUEST_PROTOCOLS:
        {
            global_outgoing_event_buffer->putEvent(SystemEventFactory::protocolPackage());
            break;
        }
        case M_REQUEST_VARIABLES:
        {
            if(global_variable_registry == NULL){
                throw SimpleException(M_SYSTEM_MESSAGE_DOMAIN, "Request received to announce all variables, "
                                                               "without a global variable registry in place");
            }
            global_variable_registry->announceAll();
            break;
        }
        case M_CLOSE_EXPERIMENT:
		{
		 Datum payload(sysEvent.getElement(M_SYSTEM_PAYLOAD));
			
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
            if (!DataFileManager::instance()->autoOpenFile()) {
                // Data file auto-open is configured, but opening the file failed.  Abort.
                merror(M_SYSTEM_MESSAGE_DOMAIN, "Failed to open data file.  Experiment will not start.");
                
                // Announce the experiment state to let the client know that the experiment didn't start
                global_outgoing_event_buffer->putEvent(SystemEventFactory::currentExperimentState());
            } else {
                mprintf(M_SYSTEM_MESSAGE_DOMAIN, "Running MWorks %s", getVersionString().c_str());
                
                time_t currentTime;
                struct tm currentLocalTime;
                std::array<char, 128> buffer;
                if (-1 != (currentTime = time(nullptr)) &&
                    localtime_r(&currentTime, &currentLocalTime) &&
                    0 != strftime(buffer.data(), buffer.size(), "%+", &currentLocalTime))
                {
                    mprintf(M_SYSTEM_MESSAGE_DOMAIN, "Current date/time is %s", buffer.data());
                }
                
                if (auto experiment = GlobalCurrentExperiment) {
                    mprintf(M_SYSTEM_MESSAGE_DOMAIN,
                            "Using protocol %s",
                            experiment->getCurrentProtocol()->getName().c_str());
                }
                
                shared_ptr <StateSystem> state_system = StateSystem::instance();
                state_system->start();
            }
		}
			break;
			
		case M_STOP_EXPERIMENT:
		{
			shared_ptr <StateSystem> state_system = StateSystem::instance();
			state_system->stop();
		}
			break;
			
		case M_PAUSE_EXPERIMENT:
		{
			shared_ptr <StateSystem> state_system = StateSystem::instance();
			state_system->pause();
		}
			break;
			
		case M_RESUME_EXPERIMENT:
		{
			shared_ptr <StateSystem> state_system = StateSystem::instance();
			state_system->resume();
		}
			break;
			
		case M_OPEN_DATA_FILE:
		{
            auto payload = sysEvent.getElement(M_SYSTEM_PAYLOAD);
            auto filename = payload.getElement(M_DATA_FILE_FILENAME).getString();
            auto overwrite = payload.getElement(M_DATA_FILE_OVERWRITE).getBool();
            if (filename.empty()) {
                merror(M_FILE_MESSAGE_DOMAIN, "Attempt to open data file with an empty name");
            } else {
                // issues an event on success
                DataFileManager::instance()->openFile(filename, overwrite);
            }
            break;
		}
		case M_CLOSE_DATA_FILE:
			DataFileManager::instance()->closeFile();
			break;
		case M_SAVE_VARIABLES:
		{
			namespace bf= boost::filesystem;
			
		 Datum payload(sysEvent.getElement(M_SYSTEM_PAYLOAD));
			
		 Datum fullPath(payload.getElement(SAVE_VARIABLES_FULL_PATH));
		 Datum file(payload.getElement(SAVE_VARIABLES_FILE));
		 Datum overwrite(payload.getElement(SAVE_VARIABLES_OVERWRITE));
			
			if(!fullPath.isInteger() || !file.isString() || !overwrite.isInteger()) {
				merror(M_STATE_SYSTEM_MESSAGE_DOMAIN,
						 "illegal property in save variables event");				
			} else {
				try {
                
                    bf::path variablesDirectory = getExperimentSavedVariablesPath(GlobalCurrentExperiment->getExperimentDirectory());
                    
                    // make sure the proper directory structure exists
                    if(bf::create_directories(variablesDirectory)) {
                        mprintf("Creating saved variables directory");
                    }
                    
                    bf::path filename;
                    if(fullPath.getBool()) {
                        filename = bf::path(file.getString());				
                    } else {
                        filename = variablesDirectory / bf::path(appendFileExtension(file.getString(), ".xml"));
                    }
                    
                    if (!bf::exists(filename) || overwrite.getBool()) {
                        if (VariableSave::saveExperimentwideVariables(filename)) {
                            GlobalCurrentExperiment->setCurrentSavedVariablesFile(filename.string());
                            global_outgoing_event_buffer->putEvent(SystemEventFactory::currentExperimentState());
                        }
                    } else {
                        mwarning(M_STATE_SYSTEM_MESSAGE_DOMAIN,
                                 "NOT overwriting current variables");
                    }

                } catch (std::exception& e){
                    merror(M_SYSTEM_MESSAGE_DOMAIN, "Error interacting with file system to save variables: %s", e.what());
                }
			}

			break;
		}	
		case M_LOAD_VARIABLES:
		{
			namespace bf= boost::filesystem;
            Datum payload(sysEvent.getElement(M_SYSTEM_PAYLOAD));
			
            Datum fullPath(payload.getElement(LOAD_VARIABLES_FULL_PATH));
            Datum file(payload.getElement(LOAD_VARIABLES_FILE));
			
			if(!fullPath.isInteger() || !file.isString()) {
				merror(M_STATE_SYSTEM_MESSAGE_DOMAIN,
					   "illegal property in load variables event");				
            } else {
                bf::path filename;
                if(fullPath.getBool()) {
                    filename = bf::path(file.getString());
                } else {
                    filename = (getExperimentSavedVariablesPath(GlobalCurrentExperiment->getExperimentDirectory()) /
                                bf::path(appendFileExtension(file.getString(), ".xml")));
                }
                
                if (VariableLoad::loadExperimentwideVariables(filename)) {
                    GlobalCurrentExperiment->setCurrentSavedVariablesFile(filename.string());
                    global_outgoing_event_buffer->putEvent(SystemEventFactory::currentExperimentState());
                }
            }
			
			break;
		}
        case M_CLIENT_CONNECTED_TO_SERVER:
        {
            // Client requested connection acknowledgement.  Respond with received client ID.
            long clientID = sysEvent.getElement(M_SYSTEM_PAYLOAD).getInteger();
            global_outgoing_event_buffer->putEvent(SystemEventFactory::serverConnectedClientResponse(clientID));
            break;
        }
		default:
			break;
	}
}


END_NAMESPACE_MW
