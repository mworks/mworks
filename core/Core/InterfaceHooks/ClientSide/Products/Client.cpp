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

typedef std::vector <shared_ptr<GenericEventFunctor> > CallbacksForVar;

Client::Client() : RegistryAwareEventStreamInterface(M_CLIENT_MESSAGE_DOMAIN, true){
	incoming_event_buffer = shared_ptr<EventBuffer>(new EventBuffer());
    outgoing_event_buffer = shared_ptr<EventBuffer>(new EventBuffer());
    
	registry = shared_ptr<VariableRegistry>(new VariableRegistry(outgoing_event_buffer));
	
	initializeStandardVariables(registry);
	message_variable = registry->getVariable(ANNOUNCE_MESSAGE_VAR_TAGNAME);
	
	
//	boost::recursive_mutex::scoped_lock lock(callbacksLock);
	for(int i = 0; i < registry->getNVariables(); ++i) {
		CallbacksForVar callbacks_for_variable;
		callbacks.push_back(callbacks_for_variable);
	}
		
	
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
	
	mprintf("reseting default client");
	
    if(remoteConnection->isConnected()) { 
		remoteConnection->disconnect();
	}
	
	return true;
}

void  Client::reset(){
	
	incoming_listener->killListener(); 
	// put the buffers back in a neutral state
	incoming_event_buffer->reset();
    outgoing_event_buffer->reset();
    
	incoming_listener->reset();
	incoming_listener->startListener();
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
	//shared_ptr<Variable> var = registry->getVariable(code);
	//var->setValue(data);
	shared_ptr<Event> event(new Event(code, data));
	putEvent(event);
}

//void Client::registerCallback(shared_ptr<GenericEventFunctor> gef) {
//// these cause esoteric locking problems
////	boost::recursive_mutex::scoped_lock lock(callbacksLock);
//	
//	for(std::vector<CallbacksForVar>::iterator i = callbacks.begin();
//		i != callbacks.end();
//		++i) {
//		i->push_back(gef);
//	}
//}
//
//void Client::registerCallback(shared_ptr<GenericEventFunctor> gef, 
//							   int code) {
//// these cause esoteric locking problems
////	boost::recursive_mutex::scoped_lock lock(callbacksLock);
//	
//	int callbacksSize = callbacks.size();
//	if(code >= 0 && code < callbacksSize) {
//		callbacks.at(code).push_back(gef);
//	} else {
//		merror(M_CLIENT_MESSAGE_DOMAIN, 
//			   "Attempting to register a callback (key: %s) for an variable code that doesn exist (code: %d)", 
//			   gef->callbackID().c_str(), 
//			   code);		
//	}
//}
//
//
//
//void Client::unregisterCallbacks(const std::string &callback_id) {
////	boost::recursive_mutex::scoped_lock lock(callbacksLock);
//	
//	// get each vector of callbacks from the main callbacks vector 
//	// (one vector for each variable)
//	for(vector<CallbacksForVar>::iterator i = callbacks.begin();
//		i != callbacks.end();
//		++i) {
//
//		vector<shared_ptr<GenericEventFunctor> > new_callbacks;
//
//		// go through the callbacks for one variable, adding each callback to a
//		// new vector except ones with "key"
//		for(vector<shared_ptr<GenericEventFunctor> >::iterator j = i->begin();
//			j != i->end();
//			++j) {
//			
//			if((*j)->callbackID() != callback_id) {
//				new_callbacks.push_back(*j);
//			}
//			
//		}
//		
//		// copy the new vector to the main callbacks list
//		i->assign(new_callbacks.begin(), new_callbacks.end());
//		
//	}
//}


END_NAMESPACE_MW
