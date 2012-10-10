/**
* Server.cpp
 *
 * History:
 * paul on 1/26/06 - Created.
 *
 * Copyright 2006 MIT. All rights reserved.
 */

#include "Experiment.h"
#include "SystemEventFactory.h"
#include "StandardSystemEventHandler.h"
#include "ExperimentPackager.h"
#include "DataFileManager.h"
#include "VariableSave.h"
#include "VariableLoad.h"

#include "Server.h"


BEGIN_NAMESPACE_MW


SINGLETON_INSTANCE_STATIC_DECLARATION(Server)


Server::Server() : RegistryAwareEventStreamInterface(M_SERVER_MESSAGE_DOMAIN, true){
	
    registry = global_variable_registry;
    
    server = shared_ptr<ScarabServer>(new ScarabServer(global_incoming_event_buffer, global_outgoing_event_buffer));

	// dont know where else this would be handled?
    if(GlobalDataFileManager == NULL) {
        GlobalDataFileManager = new DataFileManager();
    }
		
	server->setServerListenLowPort(19989);
    server->setServerListenHighPort(19999);
	server->setServerHostname("127.0.0.1");
}

Server::~Server() {
	if(incomingListener) {
		incomingListener->killListener();
	}
	if(outgoingListener) {
		outgoingListener->killListener();
	}
	if(server) {
        server->shutdown();
    }
}


bool Server::startServer() {
    
    shared_ptr<EventStreamInterface> _handler(new StandardSystemEventHandler());
	// TODO: prevents there from being more than one server instance
    incomingListener = shared_ptr<EventListener>(new EventListener(global_incoming_event_buffer, _handler));
    outgoingListener = shared_ptr<EventListener>(new EventListener(global_outgoing_event_buffer, shared_from_this()));
    
    
	outgoingListener->startListener();
    NetworkReturn *lastNetworkReturn = server->startListening();
    if(lastNetworkReturn->wasSuccessful()) {
        lastNetworkReturn = server->startAccepting();
    } else { 
		return false; 
	}
    // start the network event listener.
    incomingListener->startListener();
    return lastNetworkReturn->wasSuccessful();
}

void Server::startAccepting() {
    // TODO I am ignoring the return
	server->startListening();
    server->startAccepting();
}

void Server::stopAccepting() {
	//server->stopListening();
    server->stopAccepting();
}

void Server::stopServer() {
    server->stopAccepting();
    server->shutdown();
}


void Server::putEvent(shared_ptr<Event> event){
    global_incoming_event_buffer->putEvent(event);
}

void Server::saveVariables(const boost::filesystem::path &file) {
	VariableSave::saveExperimentwideVariables(file);
}

void Server::loadVariables(const boost::filesystem::path &file) {
	VariableLoad::loadExperimentwideVariables(file);
}

bool Server::openExperiment(const std::string &expPath) {
	namespace bf = boost::filesystem;
	
    ExperimentPackager packer;
	
    Datum experiment(packer.packageExperiment(bf::path(expPath)));
	if(experiment.isUndefined()) {
		merror(M_SERVER_MESSAGE_DOMAIN, 
			   "Failed to create a valid packaged experiment.");
		return false; 		
	}
	shared_ptr<Event> new_experiment(new Event(RESERVED_SYSTEM_EVENT_CODE, 
												 experiment));
	putEvent(new_experiment);
	
	return true;
}

bool Server::closeExperiment(){
	if(!GlobalCurrentExperiment){
		return false;
	}
	
	string expName(GlobalCurrentExperiment->getExperimentName());

	putEvent(SystemEventFactory::closeExperimentControl(expName));
	return true;
}

void Server::startDataFileManager() {
    if(GlobalDataFileManager == NULL) {
        GlobalDataFileManager = new DataFileManager();
    }
}

bool Server::openDataFile(const char * path, int options) {
    putEvent(SystemEventFactory::dataFileOpenControl(path, M_OVERWRITE));
    return true;
}

void Server::closeFile() {
    // TODO this could cause some problems.... but since the data file manager
    // is a global this and the event handler doesnt care about the name
    // this will be file for now.
    putEvent(SystemEventFactory::closeDataFileControl(""));
}

bool Server::isDataFileOpen() {
    return GlobalDataFileManager->isFileOpen();
}

bool Server::isExperimentRunning() {
	shared_ptr <StateSystem> state_system = StateSystem::instance();
	return state_system->isRunning();
}

void Server::startExperiment() {
	putEvent(SystemEventFactory::startExperimentControl());        
}

void Server::stopExperiment() {
	putEvent(SystemEventFactory::stopExperimentControl());        
}

MWTime Server::getReferenceTime() {
	shared_ptr <Clock> clock = Clock::instance();
	return clock->getSystemReferenceTime();
}

//shared_ptr<Variable> Server::getVariable(const std::string &tag) {
//	return global_variable_registry->getVariable(tag);
//}
//
//shared_ptr<Variable> Server::getVariable(const int code) {
//	return global_variable_registry->getVariable(code);
//}
//
//int Server::getCode(const std::string &tag) {
//	shared_ptr<Variable> var = getVariable(tag);
//	if(var) {
//		return var->getCodecCode();
//	} else {
//		return -1;
//	}
//}
//
//std::vector<std::string> Server::getVariableNames() {
//	return global_variable_registry->getVariableTagnames();
//}

void Server::handleEvent(shared_ptr<Event> evt) {
	handleCallbacks(evt);
}


void Server::setListenLowPort(const int port) {
	server->setServerListenLowPort(port);
}

void Server::setListenHighPort(const int port) {
	server->setServerListenHighPort(port);
}

void Server::setHostname(const std::string &name) {
	server->setServerHostname(name);
}

bool Server::isStarted() {
	return server->isConnected();
}

bool Server::isAccepting() {
	return server->isActive();
}

bool Server::isExperimentLoaded() {
	return (GlobalCurrentExperiment.use_count() > 0) ;
}


END_NAMESPACE_MW
