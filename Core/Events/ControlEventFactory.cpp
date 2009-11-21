/**
* ControlEventFactory.cpp
 *
 * History:
 * Paul Jankunas on 5/3/05 - Created.
 * Paul Jankunas on 07/07/05 - Changed string ownership for the argument passed
 *      to protocolSelectionControlEvent.  The method now creates a new string
 *      and puts that into a scarab object.
 *
 * Copyright 2005 MIT. All rights reserved.
 */

#include "ControlEventFactory.h"
#include "Experiment.h"
#include "Event.h"
#include "DataFileManager.h"
#include "VariableLoad.h"
#include "VariableSave.h"
#include <boost/filesystem/operations.hpp>
#include "PlatformDependentServices.h"
#include "ComponentRegistry.h"
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

//const int SET_EVENT_FORWARDING_PAYLOAD_SIZE = 3;
//const int SET_EVENT_FORWARDING_CODE         = 1;
//const int SET_EVENT_FORWARDING_STATE        = 2;


/*********************************************************
*          Datum Package Events
********************************************************/
shared_ptr<Event> ControlEventFactory::codecPackage() {
	return codecPackage(global_variable_registry);
}


shared_ptr<Event> ControlEventFactory::codecPackage(shared_ptr<VariableRegistry> registry) {
	shared_ptr<Event> ret(new Event(RESERVED_CODEC_CODE, registry->generateCodecDatum()));
	return ret;
}


shared_ptr<Event> ControlEventFactory::componentCodecPackage(){
	shared_ptr<ComponentRegistry> reg = ComponentRegistry::getSharedRegistry();
	shared_ptr<Event> ret(new Event(RESERVED_COMPONENT_CODEC_CODE, reg->getComponentCodec()));
	
	return ret;
}



shared_ptr<Event> ControlEventFactory::protocolPackage() {

 Datum payload(M_DICTIONARY, 3);
	
	
    if(GlobalCurrentExperiment == 0) {
		payload.addElement(M_EXPERIMENT_NAME, "No experiment loaded");
		
	 Datum empty_list(M_LIST, 1);
		payload.addElement(M_PROTOCOLS, empty_list);
		payload.addElement(M_CURRENT_PROTOCOL, "No protocol currently available");
		
		// TODO: what is this GlobalSystemEventVariable about?
		shared_ptr<Event> empty_ret(new Event(GlobalSystemEventVariable->getCodecCode(), 
									  payload));
		return empty_ret;
    }
	
    shared_ptr< vector< shared_ptr<State> > > protList = GlobalCurrentExperiment->getList();
	
	// get the experiment name
 Datum expName(GlobalCurrentExperiment->getExperimentName());
	payload.addElement(M_EXPERIMENT_NAME, expName);
	
	
	// set the protocol list
 Datum protocolList(M_LIST, (int)protList->size());
	
    for(unsigned int i = 0; i < protList->size(); ++i) {
	 Datum protocolEntry(M_DICTIONARY, 2);
		
		shared_ptr <State> protocol = (*protList)[i];
	 Datum protocolName(protocol->getName());
		protocolEntry.addElement(M_PROTOCOL_NAME, protocolName);
		
	 Datum protocolDescription(protocol->getDescription());
		protocolEntry.addElement(M_DESCRIPTION, protocolName);
		
		protocolList.addElement(protocolEntry);
	}
	
	payload.addElement(M_PROTOCOLS, protocolList);
	
	
	// set the selected protocol
	
 Datum currentProtocol(GlobalCurrentExperiment->getCurrentProtocol()->getName());
	payload.addElement(M_CURRENT_PROTOCOL, currentProtocol);
	
	
    Datum protocolPayload(systemEventPackage(M_SYSTEM_DATA_PACKAGE, 
											 M_PROTOCOL_PACKAGE, 
											 payload));
	
	shared_ptr<Event> ret(new Event(GlobalSystemEventVariable->getCodecCode(), 
									  protocolPayload));
	return ret;
}

/*********************************************************
*          Control Package Events
********************************************************/ 
shared_ptr<Event> ControlEventFactory::protocolSelectionControl(std::string  n) {
    
    Datum payload(systemEventPackage(M_SYSTEM_CONTROL_PACKAGE, 
									 M_PROTOCOL_SELECTION,
									 n));
	shared_ptr<Event> ret(new Event(GlobalSystemEventVariable->getCodecCode(), 
									  payload));
	return ret;
}

shared_ptr<Event> ControlEventFactory::startExperimentControl() {
    Datum payload(systemEventPackage(M_SYSTEM_CONTROL_PACKAGE, 
									 M_START_EXPERIMENT));
	shared_ptr<Event> ret(new Event(GlobalSystemEventVariable->getCodecCode(), 
									  payload));
	
	return ret;
}

shared_ptr<Event> ControlEventFactory::stopExperimentControl() {
    Datum payload(systemEventPackage(M_SYSTEM_CONTROL_PACKAGE, 
									 M_STOP_EXPERIMENT));
	shared_ptr<Event> ret(new Event(GlobalSystemEventVariable->getCodecCode(), 
									  payload));
	
	return ret;
}

shared_ptr<Event> ControlEventFactory::pauseExperimentControl() {
    Datum payload(systemEventPackage(M_SYSTEM_CONTROL_PACKAGE, 
									 M_PAUSE_EXPERIMENT));
	shared_ptr<Event> ret(new Event(GlobalSystemEventVariable->getCodecCode(), 
									  payload));
	
	return ret;
}

shared_ptr<Event> ControlEventFactory::requestCodecControl() {
    Datum payload(systemEventPackage(M_SYSTEM_CONTROL_PACKAGE, 
                                    M_REQUEST_CODEC));
	shared_ptr<Event> ret(new Event(GlobalSystemEventVariable->getCodecCode(), 
                                    payload));
	
	return ret;
}

shared_ptr<Event> ControlEventFactory::setEventForwardingControl(string name, bool state) {
    Datum control_datum(M_DICTIONARY, 2);
	
	control_datum.addElement(M_SET_EVENT_FORWARDING_NAME, name);
	control_datum.addElement(M_SET_EVENT_FORWARDING_STATE, state);
    
    Datum payload(systemEventPackage(M_SYSTEM_CONTROL_PACKAGE, 
                                    M_SET_EVENT_FORWARDING,
                                    control_datum));
    
    
    shared_ptr<Event> ret(new Event(GlobalSystemEventVariable->getCodecCode(), 
                                    payload));
    
	
	return ret;
}

shared_ptr<Event> ControlEventFactory::dataFileOpenControl(std::string  filename, 
													  DatumFileOptions opt) {
    
 Datum dfDatum(M_DICTIONARY, DATA_FILE_OPEN_PAYLOAD_SIZE);
	
	dfDatum.addElement(DATA_FILE_FILENAME, filename);
	dfDatum.addElement(DATA_FILE_OPTIONS, (long)opt);
	
    Datum payload(systemEventPackage(M_SYSTEM_CONTROL_PACKAGE, 
									 M_OPEN_DATA_FILE, 
									 dfDatum));
	shared_ptr<Event> ret(new Event(GlobalSystemEventVariable->getCodecCode(), 
									  payload));
	
	return ret;
}

shared_ptr<Event> ControlEventFactory::closeDataFileControl(std::string filename) {
 Datum dfDatum(M_DICTIONARY, DATA_FILE_CLOSE_PAYLOAD_SIZE);
	dfDatum.addElement(DATA_FILE_FILENAME, filename);
	
    Datum payload(systemEventPackage(M_SYSTEM_CONTROL_PACKAGE, 
									 M_CLOSE_DATA_FILE, 
									 dfDatum));
	shared_ptr<Event> ret(new Event(GlobalSystemEventVariable->getCodecCode(), 
									  payload));
	
	return ret;
}

shared_ptr<Event> ControlEventFactory::closeExperimentControl(std::string n) {
    Datum payload(systemEventPackage(M_SYSTEM_CONTROL_PACKAGE, 
									 M_CLOSE_EXPERIMENT,
									 Datum(n)));
	shared_ptr<Event> ret(new Event(GlobalSystemEventVariable->getCodecCode(), 
									  payload));
	
	return ret;
}

shared_ptr<Event> ControlEventFactory::saveVariablesControl(const std::string &file,
													   const bool _overwrite,
													   const bool fullPath) {

    Datum filename(file);
 Datum fullFilePath(fullPath);
 Datum overwrite(_overwrite);
	
 Datum svData(M_DICTIONARY, 3);
	svData.addElement(SAVE_VARIABLES_FILE,file);
	svData.addElement(SAVE_VARIABLES_FULL_PATH,fullPath);
	svData.addElement(SAVE_VARIABLES_OVERWRITE, overwrite);
	
 Datum payload(systemEventPackage(M_SYSTEM_CONTROL_PACKAGE, 
									 M_SAVE_VARIABLES,
									 svData));
	shared_ptr<Event> ret(new Event(GlobalSystemEventVariable->getCodecCode(), 
									  payload));
	
	return ret;
}

shared_ptr<Event> ControlEventFactory::loadVariablesControl(const std::string &file,
													   const bool fullPath) {
 Datum filename(file);
 Datum fullFilePath(fullPath);
	
 Datum lvData(M_DICTIONARY, 2);
	lvData.addElement(LOAD_VARIABLES_FILE,file);
	lvData.addElement(LOAD_VARIABLES_FULL_PATH,fullPath);
	
    Datum payload(systemEventPackage(M_SYSTEM_CONTROL_PACKAGE, 
									 M_LOAD_VARIABLES,
									 lvData));
	shared_ptr<Event> ret(new Event(GlobalSystemEventVariable->getCodecCode(), 
									  payload));
	
	return ret;
}

/*********************************************************
*          Response Package Events
********************************************************/ 
shared_ptr<Event> ControlEventFactory::dataFileOpenedResponse(std::string  fname,
														 SystemEventResponseCode code) {
    ScarabDatum * load, *valueDatum;
    load = scarab_list_new(DATA_FILE_OPEN_RESPONSE_PAYLOAD_SIZE);
	
    valueDatum = scarab_new_integer( code);
    scarab_list_put(load, RESPONSE_EVENT_CODE_INDEX, valueDatum);
    scarab_free_datum(valueDatum);
	
    valueDatum = scarab_new_string( fname.c_str());
    scarab_list_put(load, DATA_FILE_OPEN_RESPONSE_FILE_INDEX, valueDatum);
    scarab_free_datum(valueDatum);
	
 Datum payload(systemEventPackage(M_SYSTEM_RESPONSE_PACKAGE, 
									 M_DATA_FILE_OPENED, 
									 load));
	
	shared_ptr<Event> ret (new Event(GlobalSystemEventVariable->getCodecCode(), 
									   payload));
	
	return ret;
}

shared_ptr<Event> ControlEventFactory::dataFileClosedResponse(std::string  fname,
														 SystemEventResponseCode code) {
    ScarabDatum * load, *valueDatum;
    load = scarab_list_new(DATA_FILE_CLOSED_RESPONSE_PAYLOAD_SIZE);
	
    valueDatum = scarab_new_integer( code);
    scarab_list_put(load, RESPONSE_EVENT_CODE_INDEX, valueDatum);
    scarab_free_datum(valueDatum);
	
    valueDatum = scarab_new_string(fname.c_str());
    scarab_list_put(load, DATA_FILE_CLOSED_RESPONSE_FILE_INDEX, valueDatum);
    scarab_free_datum(valueDatum);
	
 Datum payload(systemEventPackage(M_SYSTEM_RESPONSE_PACKAGE, 
									 M_DATA_FILE_CLOSED, 
									 load));
	
	shared_ptr<Event> ret (new Event(GlobalSystemEventVariable->getCodecCode(), 
									   payload));
	
	return ret;
}



shared_ptr<Event> ControlEventFactory::serverConnectedClientResponse() {
    Datum payload(systemEventPackage(M_SYSTEM_RESPONSE_PACKAGE, 
									 M_SERVER_CONNECTED_CLIENT));
	shared_ptr<Event> ret (new Event(GlobalSystemEventVariable->getCodecCode(), 
									   payload));
	
	return ret;
}

shared_ptr<Event> ControlEventFactory::serverDisconnectClientResponse() {
    Datum payload(systemEventPackage(M_SYSTEM_RESPONSE_PACKAGE, 
									 M_SERVER_DISCONNECTED_CLIENT));
	shared_ptr<Event> ret (new Event(GlobalSystemEventVariable->getCodecCode(), 
									   payload));
	
	return ret;
}

shared_ptr<Event> ControlEventFactory::clientConnectedToServerResponse() {
    Datum payload(systemEventPackage(M_SYSTEM_RESPONSE_PACKAGE, 
									 M_CLIENT_CONNECTED_TO_SERVER));
	shared_ptr<Event> ret (new Event(GlobalSystemEventVariable->getCodecCode(), 
									   payload));
	
	return ret;
}

shared_ptr<Event> ControlEventFactory::clientDisconnectedFromServerResponse() {
    Datum payload(systemEventPackage(M_SYSTEM_RESPONSE_PACKAGE, 
									 M_CLIENT_DISCONNECTED_FROM_SERVER));
	shared_ptr<Event> ret (new Event(GlobalSystemEventVariable->getCodecCode(), 
									   payload));
	
	return ret;
}

shared_ptr<Event> ControlEventFactory::currentExperimentState() {
 Datum payload(M_DICTIONARY, 5);
	
	bool loaded = true;
	if(GlobalCurrentExperiment == NULL || GlobalCurrentExperiment.use_count() == 0){
		loaded = false;
	}
 Datum isLoaded(M_BOOLEAN, loaded);
	payload.addElement(M_LOADED, isLoaded);
	
    if(isLoaded.getBool()) {
	 Datum experimentName(GlobalCurrentExperiment->getExperimentName());
		payload.addElement(M_EXPERIMENT_NAME, experimentName);
		
		shared_ptr <StateSystem> state_system = StateSystem::instance();

        Datum isRunning(M_BOOLEAN, state_system->isRunning());
		payload.addElement(M_RUNNING, isRunning);
		
	 Datum isPaused(M_BOOLEAN, state_system->isPaused()); // can it be paused yet?
		payload.addElement(M_PAUSED, isPaused);
	
	 Datum experimentPathDatum(GlobalCurrentExperiment->getExperimentPath()); // can it be paused yet?
		payload.addElement(M_EXPERIMENT_PATH, experimentPathDatum);
	
		
		// get all of the possible saved variable sets and pack them up for sending
		namespace bf = boost::filesystem;
	
		// make sure the proper directory structure exists
		bf::path experimentPath = bf::path(GlobalCurrentExperiment->getExperimentPath(), bf::native);
		bf::path variablesDirectory = experimentPath / SAVED_VARIABLES_DIR_NAME;
		
		if(bf::exists(variablesDirectory) && bf::is_directory(variablesDirectory)) {
		 Datum savedVarList(M_LIST,1);
			
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
 Datum response(systemEventPackage(M_SYSTEM_RESPONSE_PACKAGE, 
									  M_EXPERIMENT_STATE, 
									  payload));
	
	shared_ptr<Event> ret (new Event(GlobalSystemEventVariable->getCodecCode(), 
									   response));
	
	return ret;
}



/*
 *  Response Events
 */
int ControlEventFactory::responseEventCommandCode(ScarabDatum * payload) {
    if(payload == NULL) { return M_COMMAND_SUCCESS; }
    if(payload->type != SCARAB_LIST) { return -1; }
    ScarabDatum * tmp = scarab_list_get(payload, RESPONSE_EVENT_CODE_INDEX);
    if(tmp->type != SCARAB_INTEGER) { return -1; }
    return tmp->data.integer;
}




// generic builder methods
Datum ControlEventFactory::systemEventPackage(SystemEventType eType, 
										SystemPayloadType pType,
									 Datum payload) {
 Datum systemEventPackage(M_DICTIONARY,
							 SCARAB_PAYLOAD_EVENT_N_TOPLEVEL_ELEMENTS);
	
	systemEventPackage.addElement(M_SYSTEM_EVENT_TYPE, (long)eType);
	systemEventPackage.addElement(M_SYSTEM_PAYLOAD_TYPE, (long)pType);
	systemEventPackage.addElement(M_SYSTEM_PAYLOAD, payload);
	
	return systemEventPackage;
}

Datum ControlEventFactory::systemEventPackage(SystemEventType eType, 
										SystemPayloadType pType) {
 Datum systemEventPackage(M_DICTIONARY,
							 SCARAB_EVENT_N_TOPLEVEL_ELEMENTS);
	
	systemEventPackage.addElement(M_SYSTEM_EVENT_TYPE, (long)eType);
	systemEventPackage.addElement(M_SYSTEM_PAYLOAD_TYPE, (long)pType);
	
	return systemEventPackage;
}


