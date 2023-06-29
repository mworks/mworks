/**
* StandardSystemEventHandler.cpp
 *
 * History:
 * paul on 1/27/06 - Created.
 *
 * Copyright 2006 MIT. All rights reserved.
 */

#include "StandardSystemEventHandler.h"

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include "DataFileManager.h"
#include "EventBuffer.h"
#include "Experiment.h"
#include "GenericData.h"
#include "LoadingUtilities.h"
#include "PlatformDependentServices.h"
#include "StateSystem.h"
#include "SystemEventFactory.h"
#include "Utilities.h"
#include "VariableLoad.h"
#include "VariableSave.h"


BEGIN_NAMESPACE_MW


StandardSystemEventHandler::StandardSystemEventHandler() :
    EventStreamInterface(M_SERVER_MESSAGE_DOMAIN)
{ }


void StandardSystemEventHandler::handleEvent(boost::shared_ptr<Event> evt) {
    const auto event_code = evt->getEventCode();
    
    if (event_code == RESERVED_SYSTEM_EVENT_CODE) {
        handleSystemEvent(evt->getData());
        return;
    }
    
    if (event_code < N_RESERVED_CODEC_CODES) {
        // a kind of event to which we won't respond
        return;
    }
    
    auto var = global_variable_registry->getVariable(event_code);
    if (var) {
        var->setValue(evt->getData());
    } else {
        merror(M_NETWORK_MESSAGE_DOMAIN, "Server received illegal event code: %d", event_code);
    }
}


void StandardSystemEventHandler::handleSystemEvent(const Datum &sysEvent) {
    auto payloadType = sysEvent.getElement(M_SYSTEM_PAYLOAD_TYPE);
    if (!payloadType.isInteger()) {
        merror(M_NETWORK_MESSAGE_DOMAIN, "badly formed system event");
        return;
    }
    
    switch (payloadType.getInteger()) {
            
        case M_EXPERIMENT_PACKAGE: {
            experimentUnpackager = std::make_unique<ExperimentUnpackager>();
            if (experimentUnpackager->unpackageExperiment(sysEvent.getElement(M_SYSTEM_PAYLOAD))) {
                requestNextMediaFile();
            } else {
                terminateExperimentUnpackaging();
            }
            break;
        }
            
        case M_MEDIA_FILE_PACKAGE: {
            if (experimentUnpackager) {
                if (experimentUnpackager->unpackageMediaFile(sysEvent.getElement(M_SYSTEM_PAYLOAD))) {
                    requestNextMediaFile();
                } else {
                    terminateExperimentUnpackaging();
                }
            }
            break;
        }
            
        case M_PROTOCOL_SELECTION: {
            auto newProtocol = sysEvent.getElement(M_SYSTEM_PAYLOAD).getString();
            if (newProtocol.empty()) {
                merror(M_PARADIGM_MESSAGE_DOMAIN, "Selected protocol is NULL");
                break;
            }
            if (auto experiment = GlobalCurrentExperiment) {
                experiment->setCurrentProtocol(newProtocol);
            }
            break;
        }
            
        case M_REQUEST_CODEC: {
            global_outgoing_event_buffer->putEvent(SystemEventFactory::codecPackage());
            break;
        }
            
        case M_REQUEST_COMPONENT_CODEC: {
            global_outgoing_event_buffer->putEvent(SystemEventFactory::componentCodecPackage());
            break;
        }
            
        case M_REQUEST_EXPERIMENT_STATE: {
            global_outgoing_event_buffer->putEvent(SystemEventFactory::currentExperimentState());
            break;
        }
            
        case M_REQUEST_PROTOCOLS: {
            global_outgoing_event_buffer->putEvent(SystemEventFactory::protocolPackage());
            break;
        }
            
        case M_REQUEST_VARIABLES: {
            if (global_variable_registry) {
                global_variable_registry->announceAll();
            } else {
                throw SimpleException(M_SYSTEM_MESSAGE_DOMAIN,
                                      "Request received to announce all variables, "
                                      "without a global variable registry in place");
            }
            break;
        }
            
        case M_CLOSE_EXPERIMENT: {
            auto expName = sysEvent.getElement(M_SYSTEM_PAYLOAD).getString();
            if (expName.empty()) {
                merror(M_SYSTEM_MESSAGE_DOMAIN, "Tried to close NULL experiment");
                break;
            }
            unloadExperiment();
            break;
        }
            
        case M_START_EXPERIMENT: {
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
                
                StateSystem::instance()->start();
            }
            break;
        }
            
        case M_STOP_EXPERIMENT: {
            StateSystem::instance()->stop();
            break;
        }
            
        case M_PAUSE_EXPERIMENT: {
            StateSystem::instance()->pause();
            break;
        }
            
        case M_RESUME_EXPERIMENT: {
            StateSystem::instance()->resume();
            break;
        }
            
        case M_OPEN_DATA_FILE: {
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
            
        case M_CLOSE_DATA_FILE: {
            DataFileManager::instance()->closeFile();
            break;
        }
            
        case M_SAVE_VARIABLES: {
            namespace bf = boost::filesystem;
            
            auto payload = sysEvent.getElement(M_SYSTEM_PAYLOAD);
            auto fullPath = payload.getElement(SAVE_VARIABLES_FULL_PATH);
            auto file = payload.getElement(SAVE_VARIABLES_FILE);
            auto overwrite = payload.getElement(SAVE_VARIABLES_OVERWRITE);
            
            if (!(fullPath.isInteger() && file.isString() && overwrite.isInteger())) {
                merror(M_STATE_SYSTEM_MESSAGE_DOMAIN, "illegal property in save variables event");
                break;
            }
            
            if (auto experiment = GlobalCurrentExperiment) {
                try {
                    auto variablesDirectory = getExperimentSavedVariablesPath(experiment->getExperimentDirectory());
                    
                    // make sure the proper directory structure exists
                    if (bf::create_directories(variablesDirectory)) {
                        mprintf("Creating saved variables directory");
                    }
                    
                    bf::path filename;
                    if (fullPath.getBool()) {
                        filename = bf::path(file.getString());
                    } else {
                        filename = variablesDirectory / bf::path(appendFileExtension(file.getString(), ".xml"));
                    }
                    
                    if (bf::exists(filename) && !overwrite.getBool()) {
                        mwarning(M_STATE_SYSTEM_MESSAGE_DOMAIN, "NOT overwriting current variables");
                    } else if (VariableSave::saveExperimentwideVariables(filename)) {
                        experiment->setCurrentSavedVariablesFile(filename.string());
                        global_outgoing_event_buffer->putEvent(SystemEventFactory::currentExperimentState());
                    }
                } catch (std::exception &e) {
                    merror(M_SYSTEM_MESSAGE_DOMAIN,
                           "Error interacting with file system to save variables: %s",
                           e.what());
                }
            }
            
            break;
        }
            
        case M_LOAD_VARIABLES: {
            namespace bf = boost::filesystem;
            
            auto payload = sysEvent.getElement(M_SYSTEM_PAYLOAD);
            auto fullPath = payload.getElement(LOAD_VARIABLES_FULL_PATH);
            auto file = payload.getElement(LOAD_VARIABLES_FILE);
            
            if (!(fullPath.isInteger() && file.isString())) {
                merror(M_STATE_SYSTEM_MESSAGE_DOMAIN, "illegal property in load variables event");
                break;
            }
            
            if (auto experiment = GlobalCurrentExperiment) {
                bf::path filename;
                if (fullPath.getBool()) {
                    filename = bf::path(file.getString());
                } else {
                    filename = (getExperimentSavedVariablesPath(experiment->getExperimentDirectory()) /
                                bf::path(appendFileExtension(file.getString(), ".xml")));
                }
                
                if (VariableLoad::loadExperimentwideVariables(filename)) {
                    experiment->setCurrentSavedVariablesFile(filename.string());
                    global_outgoing_event_buffer->putEvent(SystemEventFactory::currentExperimentState());
                }
            }
            
            break;
        }
            
        case M_CLIENT_CONNECTED_TO_SERVER: {
            // Client requested connection acknowledgement.  Respond with received client ID.
            auto clientID = sysEvent.getElement(M_SYSTEM_PAYLOAD).getInteger();
            global_outgoing_event_buffer->putEvent(SystemEventFactory::serverConnectedClientResponse(clientID));
            break;
        }
            
        default: {
            break;
        }
            
    }
}


void StandardSystemEventHandler::terminateExperimentUnpackaging() {
    merror(M_NETWORK_MESSAGE_DOMAIN, "Unable to unpackage experiment");
    global_outgoing_event_buffer->putEvent(SystemEventFactory::currentExperimentState());
    experimentUnpackager.reset();
}


void StandardSystemEventHandler::requestNextMediaFile() {
    auto mediaFileRequestPayload = experimentUnpackager->createMediaFileRequest();
    if (!mediaFileRequestPayload.isUndefined()) {
        // Request the next media file
        global_outgoing_event_buffer->putEvent(boost::make_shared<Event>(RESERVED_SYSTEM_EVENT_CODE,
                                                                         mediaFileRequestPayload));
    } else {
        // No more media files required.  Load the experiment.
        auto expXMLFile = experimentUnpackager->getExperimentFilePath();
        if (!loadExperimentFromXMLParser(expXMLFile)) {
            merror(M_PARSER_MESSAGE_DOMAIN, "Failed to parse experiment %s", expXMLFile.string().c_str());
        } else if (auto experiment = GlobalCurrentExperiment) {
            mprintf("%s successfully loaded, using protocol: %s",
                    experiment->getExperimentName().c_str(),
                    experiment->getCurrentProtocol()->getName().c_str());
        }
        experimentUnpackager.reset();
    }
}


END_NAMESPACE_MW
