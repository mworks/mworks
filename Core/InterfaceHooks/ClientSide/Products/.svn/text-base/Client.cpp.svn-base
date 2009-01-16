/**
* DefaultClient.cpp
 *
 * History:
 * paul on 1/26/06 - Created.
 *
 * Copyright 2006 MIT. All rights reserved.
 */

#include "Client.h"
#include "ExperimentPackager.h"
#include "EventFactory.h"
#include "LoadingUtilities.h"
#include "boost/filesystem/path.hpp"
using namespace mw;

#define CLIENT_INTERNAL_KEY	"MWClientInternalCallbackKey"

typedef std::vector <shared_ptr<GenericEventFunctor> > CallbacksForVar;

Client::Client() {
	buffer_manager = shared_ptr<BufferManager>(new BufferManager());

	registry = shared_ptr<VariableRegistry>(new VariableRegistry(buffer_manager));
	
	initializeStandardVariables(registry);
	system_event_variable = registry->getVariable(SYSTEM_VAR_TAGNAME);
	message_variable = registry->getVariable(ANNOUNCE_MESSAGE_VAR_TAGNAME);
	
	
//	boost::recursive_mutex::scoped_lock lock(callbacksLock);
	for(int i = 0; i < registry->getNVariables(); ++i) {
		CallbacksForVar callbacks_for_variable;
		callbacks.push_back(callbacks_for_variable);
	}
		
	
	// TODO: shared_ptr to "this" is janked
    incomingListener = shared_ptr<IncomingEventListener>(new IncomingEventListener(buffer_manager, shared_ptr<EventHandler>(this)));

	
}


Client::~Client() {
    if(remoteConnection) {
        disconnectClient();
    }
	
    if(incomingListener) {
        incomingListener->killListener();
    }
}



void Client::handleEvent(shared_ptr<Event> &evt) { 
	int code = evt->getEventCode();
	
	if(code == RESERVED_CODEC_CODE) {
		registry->update(evt->getData());
		
		CallbacksForVar callbacks_for_variable;
		{
			// this is ony locked in this function and it is not reentrant
//			boost::recursive_mutex::scoped_lock lock(callbacksLock);
			CallbacksForVar empty_callback_list;
			callbacks.resize(registry->getNVariables(), empty_callback_list);		
			callbacks_for_variable = callbacks.at(code);
		}
		
		for(CallbacksForVar::iterator i = callbacks_for_variable.begin();
			i!=callbacks_for_variable.end(); 
			++i) {
			
			(*i)->operator()(evt);
		}
		
	} else {
		// this is ony locked in this function and it is not reentrant
//		boost::recursive_mutex::scoped_lock lock(callbacksLock);

		if(code > 0 && code < registry->getNVariables() && code < (int)callbacks.size()) {
			shared_ptr <Variable> var = registry->getVariable(code);
			if(var) {
				var->setSilentValue(evt->getData());
				
				if(callbacks.at(code).size() > 0) {
					for(CallbacksForVar::iterator i = callbacks.at(code).begin();
						i!=callbacks.at(code).end(); 
						++i) {
						
						(*i)->operator()(evt);
					}
				}
			}
		} else {
			mwarning(M_CLIENT_MESSAGE_DOMAIN, "Invalid variable, client not processing event code: %d", code);
		}
	}
}

void Client::startEventListener(){
	incomingListener->startListener();
}

bool Client::connectToServer(const std::string &host, const int port) {
    remoteConnection = shared_ptr<ScarabClient>(new ScarabClient(buffer_manager, host, port));
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
	
	incomingListener->killListener(); 
	// put the buffers back in a neutral state
	buffer_manager->reset();
	incomingListener->reset();
	incomingListener->startListener();
}

bool Client::isConnected() {
    if(NULL == remoteConnection) { return false; }
    return remoteConnection->isConnected();
}

bool Client::sendExperiment(const std::string &expPath) {
	namespace bf = boost::filesystem;
    if(!remoteConnection->isConnected()) { return false; }
    ExperimentPackager packer;
	
	Data experiment(packer.packageExperiment(bf::path(expPath, bf::native)));
	if(experiment.isUndefined()) {
		merror(M_CLIENT_MESSAGE_DOMAIN, 
			   "Failed to create a valid packaged experiment.");
		
		// send an update that the experiment load failed
		shared_ptr<Event> experimentStateEvent = EventFactory::currentExperimentState();
		buffer_manager->putEvent(experimentStateEvent);
			
		return false; 		
	}
	system_event_variable->setValue(experiment);
	
    return true;
}

void  Client::sendProtocolSelectedEvent(const std::string &protocolname) {
    if(protocolname.size() == 0) { return; }
    buffer_manager->putEvent(EventFactory::protocolSelectionControl(protocolname));
}

void  Client::sendRunEvent() {
    buffer_manager->putEvent(EventFactory::startExperimentControl());
}

void  Client::sendStopEvent() {
    buffer_manager->putEvent(EventFactory::stopExperimentControl());
}

void  Client::sendPauseEvent() {
    buffer_manager->putEvent(EventFactory::pauseExperimentControl());
}

void  Client::sendUnpauseEvent() {
    buffer_manager->putEvent(EventFactory::pauseExperimentControl());
}

void  Client::sendOpenDataFileEvent(const std::string &filename, 
									 const int options) {
    if(filename.size() == 0) { return; }
	DataFileOptions overwrite = options ? M_OVERWRITE : M_NO_OVERWRITE;
    buffer_manager->putEvent(EventFactory::dataFileOpenControl(filename, 
																	 overwrite));
}

void  Client::sendCloseDataFileEvent(const std::string &filename) {
    buffer_manager->putEvent(EventFactory::closeDataFileControl(filename));
}

void  Client::sendCloseExperimentEvent(const std::string &expName) {
    if(expName.size() == 0) { return; }
    buffer_manager->putEvent(EventFactory::closeExperimentControl(expName));
}

void  Client::sendSaveVariablesEvent(const std::string &file, const bool overwrite) {
    buffer_manager->putEvent(EventFactory::saveVariablesControl(file, overwrite, false));
}

void  Client::sendLoadVariablesEvent(const std::string &file) {
    buffer_manager->putEvent(EventFactory::loadVariablesControl(file, false));
}

void Client::updateRegistry(const Data &codec) {
	registry->update(codec);
}

shared_ptr<Variable> Client::getVariable(const std::string &tag) {
	return registry->getVariable(tag);
}

shared_ptr<Variable> Client::getVariable(const int code) {
	return registry->getVariable(code);
}

int Client::getCode(const std::string &tag) {
	shared_ptr<Variable> var = getVariable(tag);
	if(var) {
		return var->getCodecCode();
	} else {
		return -1;
	}
}

std::vector<std::string> Client::getVariableNames() {
	return registry->getVariableTagnames();
}

void Client::updateValue(const int code, const Data &data) {
	//shared_ptr<Variable> var = registry->getVariable(code);
	//var->setValue(data);
	shared_ptr<Event> event(new Event(code, data));
	buffer_manager->putEvent(event);
}

void Client::registerCallback(shared_ptr<GenericEventFunctor> gef) {
// these cause esoteric locking problems
//	boost::recursive_mutex::scoped_lock lock(callbacksLock);
	
	for(std::vector<CallbacksForVar>::iterator i = callbacks.begin();
		i != callbacks.end();
		++i) {
		i->push_back(gef);
	}
}

void Client::registerCallback(shared_ptr<GenericEventFunctor> gef, 
							   int code) {
// these cause esoteric locking problems
//	boost::recursive_mutex::scoped_lock lock(callbacksLock);
	
	int callbacksSize = callbacks.size();
	if(code >= 0 && code < callbacksSize) {
		callbacks.at(code).push_back(gef);
	} else {
		merror(M_CLIENT_MESSAGE_DOMAIN, 
			   "Attempting to register a callback (key: %s) for an variable code that doesn exist (code: %d)", 
			   gef->callbackID().c_str(), 
			   code);		
	}
}

void Client::registerCallback(shared_ptr<GenericEventFunctor> gef,
							   string tagname) {
	int code = getCode(tagname);
	registerCallback(gef, code);
}

void Client::unregisterCallbacks(const std::string &callback_id) {
	using namespace std;
	
//	boost::recursive_mutex::scoped_lock lock(callbacksLock);
	
	// get each vector of callbacks from the main callbacks vector 
	// (one vector for each variable)
	for(vector<CallbacksForVar>::iterator i = callbacks.begin();
		i != callbacks.end();
		++i) {

		vector<shared_ptr<GenericEventFunctor> > new_callbacks;

		// go through the callbacks for one variable, adding each callback to a
		// new vector except ones with "key"
		for(vector<shared_ptr<GenericEventFunctor> >::iterator j = i->begin();
			j != i->end();
			++j) {
			
			if((*j)->callbackID() != callback_id) {
				new_callbacks.push_back(*j);
			}
			
		}
		
		// copy the new vector to the main callbacks list
		i->assign(new_callbacks.begin(), new_callbacks.end());
		
	}
}

