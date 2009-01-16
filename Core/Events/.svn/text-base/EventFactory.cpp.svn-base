/**
* EventFactory.cpp
 *
 * History:
 * Paul Jankunas on 5/3/05 - Created.
 * Paul Jankunas on 07/07/05 - Changed string ownership for the argument passed
 *      to protocolSelectionControlEvent.  The method now creates a new string
 *      and puts that into a scarab object.
 *
 * Copyright 2005 MIT. All rights reserved.
 */

#include "EventFactory.h"
#include "Experiment.h"
#include "Event.h"
#include "DataFileManager.h"
#include "VariableLoad.h"
#include "VariableSave.h"
#include <boost/filesystem/operations.hpp>
#include "PlatformDependentServices.h"
#include "ComponentRegistry_new.h"
using namespace mw;

const int AUTOMATIC_RESPONSE_EVENT_PAYLOAD_SIZE     = 1;
const int RESPONSE_EVENT_CODE_INDEX                 = 0;

// Package Event Package Constants
// how many extra spaces are added to the protocol list
const int PROTOCOL_PACAKGE_MODIFIER                 = 4;
const int PROTOCOL_PACKAGE_SIZE_INDEX               = 0;
const int PROTOCOL_PACKAGE_EXPERIMENT_NAME_INDEX    = 1;
const int PROTOCOL_PACKAGE_PROTOCOL_NAME_INDEX      = 2;
const int PROTOCOL_PACKAGE_EMPTY_INDEX              = 3;

// Control Event Package Constants
const int DATA_FILE_OPEN_PAYLOAD_SIZE           = 2;
const int DATA_FILE_CLOSE_PAYLOAD_SIZE          = 1;

// Response Event Package Constants
const int EXPERIMENT_LOADED_RESPONSE_PAYLOAD_SIZE   = 3;
const int EXPERIMENT_LOADED_RESPONSE_NAME_INDEX     = 1;
const int EXPERIMENT_LOADED_RESPONSE_MSG_INDEX      = 2;

const int EXPERIMENT_UNLOADED_RESPONSE_PAYLOAD_SIZE   = 3;
const int EXPERIMENT_UNLOADED_RESPONSE_NAME_INDEX     = 1;
const int EXPERIMENT_UNLOADED_RESPONSE_MSG_INDEX      = 2;

const int DATA_FILE_OPEN_RESPONSE_PAYLOAD_SIZE  = 2;
const int DATA_FILE_OPEN_RESPONSE_FILE_INDEX    = 1;

const int DATA_FILE_CLOSED_RESPONSE_PAYLOAD_SIZE    = 2;
const int DATA_FILE_CLOSED_RESPONSE_FILE_INDEX      = 1;


/*********************************************************
*          Data Package Events
********************************************************/
shared_ptr<Event> EventFactory::codecPackage() {
	return codecPackage(GlobalVariableRegistry);
}


shared_ptr<Event> EventFactory::codecPackage(shared_ptr<VariableRegistry> registry) {
	shared_ptr<Event> ret(new Event(RESERVED_CODEC_CODE, registry->getCodec()));
	return ret;
}


shared_ptr<Event> EventFactory::componentCodecPackage(){
	shared_ptr<mwComponentRegistry> reg = mwComponentRegistry::getSharedRegistry();
	shared_ptr<Event> ret(new Event(RESERVED_COMPONENT_CODEC_CODE, reg->getComponentCodec()));
	
	return ret;
}



shared_ptr<Event> EventFactory::protocolPackage() {

	Data payload(M_DICTIONARY, 3);
	
	
    if(GlobalCurrentExperiment == 0) {
		payload.addElement(M_EXPERIMENT_NAME, "No experiment loaded");
		
		Data empty_list(M_LIST, 1);
		payload.addElement(M_PROTOCOLS, empty_list);
		payload.addElement(M_CURRENT_PROTOCOL, "No protocol currently available");
		
		// TODO: what is this GlobalSystemEventVariable about?
		shared_ptr<Event> empty_ret(new Event(GlobalSystemEventVariable->getCodecCode(), 
									  payload));
		return empty_ret;
    }
	
    shared_ptr< vector< shared_ptr<State> > > protList = GlobalCurrentExperiment->getList();
	
	// get the experiment name
	Data expName(GlobalCurrentExperiment->getExperimentName());
	payload.addElement(M_EXPERIMENT_NAME, expName);
	
	
	// set the protocol list
	Data protocolList(M_LIST, (int)protList->size());
	
    for(unsigned int i = 0; i < protList->size(); ++i) {
		Data protocolEntry(M_DICTIONARY, 2);
		
		shared_ptr <State> protocol = (*protList)[i];
		Data protocolName(protocol->getName());
		protocolEntry.addElement(M_PROTOCOL_NAME, protocolName);
		
		Data protocolDescription(protocol->getDescription());
		protocolEntry.addElement(M_DESCRIPTION, protocolName);
		
		protocolList.addElement(protocolEntry);
	}
	
	payload.addElement(M_PROTOCOLS, protocolList);
	
	
	// set the selected protocol
	
	Data currentProtocol(GlobalCurrentExperiment->getCurrentProtocol()->getName());
	payload.addElement(M_CURRENT_PROTOCOL, currentProtocol);
	
	
    Data protocolPayload(systemEventPackage(M_SYSTEM_DATA_PACKAGE, 
											 M_PROTOCOL_PACKAGE, 
											 payload));
	
	shared_ptr<Event> ret(new Event(GlobalSystemEventVariable->getCodecCode(), 
									  protocolPayload));
	return ret;
}

/*********************************************************
*          Control Package Events
********************************************************/ 
shared_ptr<Event> EventFactory::protocolSelectionControl(std::string  n) {
    
    Data payload(systemEventPackage(M_SYSTEM_CONTROL_PACKAGE, 
									 M_PROTOCOL_SELECTION,
									 n));
	shared_ptr<Event> ret(new Event(GlobalSystemEventVariable->getCodecCode(), 
									  payload));
	return ret;
}

shared_ptr<Event> EventFactory::startExperimentControl() {
    Data payload(systemEventPackage(M_SYSTEM_CONTROL_PACKAGE, 
									 M_START_EXPERIMENT));
	shared_ptr<Event> ret(new Event(GlobalSystemEventVariable->getCodecCode(), 
									  payload));
	
	return ret;
}

shared_ptr<Event> EventFactory::stopExperimentControl() {
    Data payload(systemEventPackage(M_SYSTEM_CONTROL_PACKAGE, 
									 M_STOP_EXPERIMENT));
	shared_ptr<Event> ret(new Event(GlobalSystemEventVariable->getCodecCode(), 
									  payload));
	
	return ret;
}

shared_ptr<Event> EventFactory::pauseExperimentControl() {
    Data payload(systemEventPackage(M_SYSTEM_CONTROL_PACKAGE, 
									 M_PAUSE_EXPERIMENT));
	shared_ptr<Event> ret(new Event(GlobalSystemEventVariable->getCodecCode(), 
									  payload));
	
	return ret;
}

shared_ptr<Event> EventFactory::dataFileOpenControl(std::string  filename, 
													  DataFileOptions opt) {
    
	Data dfDatum(M_DICTIONARY, DATA_FILE_OPEN_PAYLOAD_SIZE);
	
	dfDatum.addElement(DATA_FILE_FILENAME, filename);
	dfDatum.addElement(DATA_FILE_OPTIONS, (long)opt);
	
    Data payload(systemEventPackage(M_SYSTEM_CONTROL_PACKAGE, 
									 M_OPEN_DATA_FILE, 
									 dfDatum));
	shared_ptr<Event> ret(new Event(GlobalSystemEventVariable->getCodecCode(), 
									  payload));
	
	return ret;
}

shared_ptr<Event> EventFactory::closeDataFileControl(std::string filename) {
	Data dfDatum(M_DICTIONARY, DATA_FILE_CLOSE_PAYLOAD_SIZE);
	dfDatum.addElement(DATA_FILE_FILENAME, filename);
	
    Data payload(systemEventPackage(M_SYSTEM_CONTROL_PACKAGE, 
									 M_CLOSE_DATA_FILE, 
									 dfDatum));
	shared_ptr<Event> ret(new Event(GlobalSystemEventVariable->getCodecCode(), 
									  payload));
	
	return ret;
}

shared_ptr<Event> EventFactory::closeExperimentControl(std::string n) {
    Data payload(systemEventPackage(M_SYSTEM_CONTROL_PACKAGE, 
									 M_CLOSE_EXPERIMENT,
									 Data(n)));
	shared_ptr<Event> ret(new Event(GlobalSystemEventVariable->getCodecCode(), 
									  payload));
	
	return ret;
}

shared_ptr<Event> EventFactory::saveVariablesControl(const std::string &file,
													   const bool _overwrite,
													   const bool fullPath) {

    Data filename(file);
	Data fullFilePath(fullPath);
	Data overwrite(_overwrite);
	
	Data svData(M_DICTIONARY, 3);
	svData.addElement(SAVE_VARIABLES_FILE,file);
	svData.addElement(SAVE_VARIABLES_FULL_PATH,fullPath);
	svData.addElement(SAVE_VARIABLES_OVERWRITE, overwrite);
	
	Data payload(systemEventPackage(M_SYSTEM_CONTROL_PACKAGE, 
									 M_SAVE_VARIABLES,
									 svData));
	shared_ptr<Event> ret(new Event(GlobalSystemEventVariable->getCodecCode(), 
									  payload));
	
	return ret;
}

shared_ptr<Event> EventFactory::loadVariablesControl(const std::string &file,
													   const bool fullPath) {
	Data filename(file);
	Data fullFilePath(fullPath);
	
	Data lvData(M_DICTIONARY, 2);
	lvData.addElement(LOAD_VARIABLES_FILE,file);
	lvData.addElement(LOAD_VARIABLES_FULL_PATH,fullPath);
	
    Data payload(systemEventPackage(M_SYSTEM_CONTROL_PACKAGE, 
									 M_LOAD_VARIABLES,
									 lvData));
	shared_ptr<Event> ret(new Event(GlobalSystemEventVariable->getCodecCode(), 
									  payload));
	
	return ret;
}

/*********************************************************
*          Response Package Events
********************************************************/ 
shared_ptr<Event> EventFactory::dataFileOpenedResponse(std::string  fname,
														 SystemEventResponseCode code) {
    ScarabDatum * load, *valueDatum;
    load = scarab_list_new(DATA_FILE_OPEN_RESPONSE_PAYLOAD_SIZE);
	
    valueDatum = scarab_new_integer( code);
    scarab_list_put(load, RESPONSE_EVENT_CODE_INDEX, valueDatum);
    scarab_free_datum(valueDatum);
	
    valueDatum = scarab_new_string( fname.c_str());
    scarab_list_put(load, DATA_FILE_OPEN_RESPONSE_FILE_INDEX, valueDatum);
    scarab_free_datum(valueDatum);
	
	Data payload(systemEventPackage(M_SYSTEM_RESPONSE_PACKAGE, 
									 M_DATA_FILE_OPENED, 
									 load));
	
	shared_ptr<Event> ret (new Event(GlobalSystemEventVariable->getCodecCode(), 
									   payload));
	
	return ret;
}

shared_ptr<Event> EventFactory::dataFileClosedResponse(std::string  fname,
														 SystemEventResponseCode code) {
    ScarabDatum * load, *valueDatum;
    load = scarab_list_new(DATA_FILE_CLOSED_RESPONSE_PAYLOAD_SIZE);
	
    valueDatum = scarab_new_integer( code);
    scarab_list_put(load, RESPONSE_EVENT_CODE_INDEX, valueDatum);
    scarab_free_datum(valueDatum);
	
    valueDatum = scarab_new_string(fname.c_str());
    scarab_list_put(load, DATA_FILE_CLOSED_RESPONSE_FILE_INDEX, valueDatum);
    scarab_free_datum(valueDatum);
	
	Data payload(systemEventPackage(M_SYSTEM_RESPONSE_PACKAGE, 
									 M_DATA_FILE_CLOSED, 
									 load));
	
	shared_ptr<Event> ret (new Event(GlobalSystemEventVariable->getCodecCode(), 
									   payload));
	
	return ret;
}



shared_ptr<Event> EventFactory::serverConnectedClientResponse() {
    Data payload(systemEventPackage(M_SYSTEM_RESPONSE_PACKAGE, 
									 M_SERVER_CONNECTED_CLIENT));
	shared_ptr<Event> ret (new Event(GlobalSystemEventVariable->getCodecCode(), 
									   payload));
	
	return ret;
}

shared_ptr<Event> EventFactory::serverDisconnectClientResponse() {
    Data payload(systemEventPackage(M_SYSTEM_RESPONSE_PACKAGE, 
									 M_SERVER_DISCONNECTED_CLIENT));
	shared_ptr<Event> ret (new Event(GlobalSystemEventVariable->getCodecCode(), 
									   payload));
	
	return ret;
}

shared_ptr<Event> EventFactory::clientConnectedToServerResponse() {
    Data payload(systemEventPackage(M_SYSTEM_RESPONSE_PACKAGE, 
									 M_CLIENT_CONNECTED_TO_SERVER));
	shared_ptr<Event> ret (new Event(GlobalSystemEventVariable->getCodecCode(), 
									   payload));
	
	return ret;
}

shared_ptr<Event> EventFactory::clientDisconnectedFromServerResponse() {
    Data payload(systemEventPackage(M_SYSTEM_RESPONSE_PACKAGE, 
									 M_CLIENT_DISCONNECTED_FROM_SERVER));
	shared_ptr<Event> ret (new Event(GlobalSystemEventVariable->getCodecCode(), 
									   payload));
	
	return ret;
}

shared_ptr<Event> EventFactory::currentExperimentState() {
	Data payload(M_DICTIONARY, 5);
	
	bool loaded = true;
	if(GlobalCurrentExperiment == NULL || GlobalCurrentExperiment.use_count() == 0){
		loaded = false;
	}
	Data isLoaded(M_BOOLEAN, loaded);
	payload.addElement(M_LOADED, isLoaded);
	
    if(isLoaded.getBool()) {
		Data experimentName(GlobalCurrentExperiment->getExperimentName());
		payload.addElement(M_EXPERIMENT_NAME, experimentName);
		
		shared_ptr <StateSystem> state_system = StateSystem::instance();

        Data isRunning(M_BOOLEAN, state_system->isRunning());
		payload.addElement(M_RUNNING, isRunning);
		
		Data isPaused(M_BOOLEAN, state_system->isPaused()); // can it be paused yet?
		payload.addElement(M_PAUSED, isPaused);
	
		Data experimentPathDatum(GlobalCurrentExperiment->getExperimentPath()); // can it be paused yet?
		payload.addElement(M_EXPERIMENT_PATH, experimentPathDatum);
	
		
		// get all of the possible saved variable sets and pack them up for sending
		namespace bf = boost::filesystem;
	
		// make sure the proper directory structure exists
		bf::path experimentPath = bf::path(GlobalCurrentExperiment->getExperimentPath(), bf::native);
		bf::path variablesDirectory = experimentPath / SAVED_VARIABLES_DIR_NAME;
		
		if(bf::exists(variablesDirectory) && bf::is_directory(variablesDirectory)) {
			Data savedVarList(M_LIST,1);
			
			bf::directory_iterator end_itr; // default construction yields past-the-end
			for ( bf::directory_iterator itr(variablesDirectory);
				  itr != end_itr;
				  ++itr ) {
				
				if (fileExtension(itr->filename()) == "xml")
				{
					savedVarList.addElement(removeFileExtension(itr->filename()));
				}
			}
			
			
			payload.addElement(M_SAVED_VARIABLES, savedVarList);
			
			
		}
		
	}
	Data response(systemEventPackage(M_SYSTEM_RESPONSE_PACKAGE, 
									  M_EXPERIMENT_STATE, 
									  payload));
	
	shared_ptr<Event> ret (new Event(GlobalSystemEventVariable->getCodecCode(), 
									   response));
	
	return ret;
}



/*
 *  Response Events
 */
int EventFactory::responseEventCommandCode(ScarabDatum * payload) {
    if(payload == NULL) { return M_COMMAND_SUCCESS; }
    if(payload->type != SCARAB_LIST) { return -1; }
    ScarabDatum * tmp = scarab_list_get(payload, RESPONSE_EVENT_CODE_INDEX);
    if(tmp->type != SCARAB_INTEGER) { return -1; }
    return tmp->data.integer;
}




// generic builder methods
Data EventFactory::systemEventPackage(SystemEventType eType, 
										SystemPayloadType pType,
										Data payload) {
	Data systemEventPackage(M_DICTIONARY,
							 SCARAB_PAYLOAD_EVENT_N_TOPLEVEL_ELEMENTS);
	
	systemEventPackage.addElement(M_SYSTEM_EVENT_TYPE, (long)eType);
	systemEventPackage.addElement(M_SYSTEM_PAYLOAD_TYPE, (long)pType);
	systemEventPackage.addElement(M_SYSTEM_PAYLOAD, payload);
	
	return systemEventPackage;
}

Data EventFactory::systemEventPackage(SystemEventType eType, 
										SystemPayloadType pType) {
	Data systemEventPackage(M_DICTIONARY,
							 SCARAB_EVENT_N_TOPLEVEL_ELEMENTS);
	
	systemEventPackage.addElement(M_SYSTEM_EVENT_TYPE, (long)eType);
	systemEventPackage.addElement(M_SYSTEM_PAYLOAD_TYPE, (long)pType);
	
	return systemEventPackage;
}


