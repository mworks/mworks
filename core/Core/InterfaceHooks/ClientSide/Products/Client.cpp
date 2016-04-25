/**
* DefaultClient.cpp
 *
 * History:
 * paul on 1/26/06 - Created.
 *
 * Copyright 2006 MIT. All rights reserved.
 */

#include "ExperimentPackager.h"
#include "Client.h"
#include "SystemEventFactory.h"
#include "LoadingUtilities.h"
#include "boost/filesystem/path.hpp"


BEGIN_NAMESPACE_MW


#define CLIENT_INTERNAL_KEY	"MWClientInternalCallbackKey"


Client::Client() : RegistryAwareEventStreamInterface(M_CLIENT_MESSAGE_DOMAIN){
	incoming_event_buffer = shared_ptr<EventBuffer>(new EventBuffer());
    outgoing_event_buffer = shared_ptr<EventBuffer>(new EventBuffer());
    
	registry = shared_ptr<VariableRegistry>(new VariableRegistry(outgoing_event_buffer));
	
	initializeStandardVariables(registry);
	message_variable = registry->getVariable(ANNOUNCE_MESSAGE_VAR_TAGNAME);
}


Client::~Client() {
    if(remoteConnection) {
        disconnectClient();
    }
	
    if(incoming_listener) {
        incoming_listener->killListener();
    }
}



void Client::handleEvent(shared_ptr<Event> evt) { 
	int code = evt->getEventCode();
	
	if(code == RESERVED_CODEC_CODE) {
		registry->updateFromCodecDatum(evt->getData());
        
        // request updated values for these new variables
        outgoing_event_buffer->putEvent( SystemEventFactory::requestVariablesUpdateControl() );
    }
    
    
    handleCallbacks(evt);

    if(code >= N_RESERVED_CODEC_CODES && code < registry->getNVariables() + N_RESERVED_CODEC_CODES) {
        shared_ptr <Variable> var = registry->getVariable(code);
        if(var) {
            var->setSilentValue(evt->getData());
		} else {
			mwarning(M_CLIENT_MESSAGE_DOMAIN, "Invalid variable, client not processing event code: %d", code);
		}
	}
}

void Client::startEventListener(){
    
    //shared_ptr<Client> this_ptr = Client::shared_from_this();
    shared_ptr<EventStreamInterface> this_as_evt_handler = shared_from_this();
    incoming_listener = shared_ptr<EventListener>(new EventListener(incoming_event_buffer, this_as_evt_handler));
    
	incoming_listener->startListener();
}

bool Client::connectToServer(const std::string &host, const int port) {
    remoteConnection = shared_ptr<ScarabClient>(new ScarabClient(incoming_event_buffer, outgoing_event_buffer, host, port));
    if(remoteConnection == NULL) { return false; }
    
	lastNetworkReturn = remoteConnection->connect();
    if(!lastNetworkReturn->wasSuccessful()) { 
        //TODO log the error somewhere.
        return false; 
    }
    remoteConnection->start();
    return true;
}

bool Client::disconnectClient() {
	
	mprintf("resetting default client");
	
    if(remoteConnection->isConnected()) { 
		remoteConnection->disconnect();
	}
	
	return true;
}

bool Client::isConnected() {
    if(NULL == remoteConnection) { return false; }
    return remoteConnection->isConnected();
}


void Client::putEvent(shared_ptr<Event> event) {
    outgoing_event_buffer->putEvent(event);
}

bool Client::sendExperiment(const std::string &expPath) {
	namespace bf = boost::filesystem;
    if(!remoteConnection->isConnected()) { return false; }
    ExperimentPackager packer;
	
    Datum packaged_experiment(packer.packageExperiment(bf::path(expPath)));
	if(packaged_experiment.isUndefined()) {
		merror(M_CLIENT_MESSAGE_DOMAIN, 
			   "Failed to create a valid packaged experiment.");
		
		// send an update that the experiment load failed
		shared_ptr<Event> experimentStateEvent = SystemEventFactory::currentExperimentState();
		putEvent(experimentStateEvent);
			
		return false; 		
	}
    
    shared_ptr<Event> experiment_event(new Event(RESERVED_SYSTEM_EVENT_CODE, packaged_experiment));
    putEvent(experiment_event);
	
    return true;
}

void  Client::sendProtocolSelectedEvent(const std::string &protocolname) {
    if(protocolname.size() == 0) { return; }
    putEvent(SystemEventFactory::protocolSelectionControl(protocolname));
}

void  Client::sendRunEvent() {
    putEvent(SystemEventFactory::startExperimentControl());
}

void  Client::sendStopEvent() {
    putEvent(SystemEventFactory::stopExperimentControl());
}

void  Client::sendPauseEvent() {
    putEvent(SystemEventFactory::pauseExperimentControl());
}

void  Client::sendResumeEvent() {
    putEvent(SystemEventFactory::resumeExperimentControl());
}

void  Client::sendOpenDataFileEvent(const std::string &filename, 
									 const int options) {
    if(filename.size() == 0) { return; }
 DatumFileOptions overwrite = options ? M_OVERWRITE : M_NO_OVERWRITE;
    putEvent(SystemEventFactory::dataFileOpenControl(filename, 
																	 overwrite));
}

void  Client::sendCloseDataFileEvent(const std::string &filename) {
    putEvent(SystemEventFactory::closeDataFileControl(filename));
}

void  Client::sendCloseExperimentEvent(const std::string &expName) {
    if(expName.size() == 0) { return; }
    putEvent(SystemEventFactory::closeExperimentControl(expName));
}

void  Client::sendSaveVariablesEvent(const std::string &file, const bool overwrite) {
    putEvent(SystemEventFactory::saveVariablesControl(file, overwrite, false));
}

void  Client::sendLoadVariablesEvent(const std::string &file) {
    putEvent(SystemEventFactory::loadVariablesControl(file, false));
}

void Client::updateRegistry(const Datum &codec) {
	registry->updateFromCodecDatum(codec);
}


void Client::updateValue(const int code, const Datum &data) {
	shared_ptr<Event> event(new Event(code, data));
	putEvent(event);
}


END_NAMESPACE_MW
