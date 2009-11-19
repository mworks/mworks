/**
* Server.cpp
 *
 * History:
 * paul on 1/26/06 - Created.
 *
 * Copyright 2006 MIT. All rights reserved.
 */

#include "Server.h"
#include "Experiment.h"
#include "EventFactory.h"
#include "DefaultEventHandler.h"
#include "ExperimentPackager.h"
#include "DataFileManager.h"
#include "VariableSave.h"
#include "VariableLoad.h"


using namespace mw;


Server::Server() : EventHandler(M_SERVER_MESSAGE_DOMAIN, true){
	
    registry = GlobalVariableRegistry;
    
    server = shared_ptr<ScarabServer>(new ScarabServer(GlobalBufferManager));

		
	
	shared_ptr<DefaultEventHandler> _handler(new DefaultEventHandler());
	// TODO: prevents there from being more than one server instance
    incomingListener = shared_ptr<IncomingEventListener>(new IncomingEventListener(GlobalBufferManager, _handler));
    outgoingListener = shared_ptr<OutgoingEventListener>(new OutgoingEventListener(GlobalBufferManager, shared_ptr<EventHandler>(this)));
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
    GlobalBufferManager->putIncomingNetworkEvent(event);
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
	
	Data experiment(packer.packageExperiment(bf::path(expPath, bf::native)));
	if(experiment.isUndefined()) {
		merror(M_SERVER_MESSAGE_DOMAIN, 
			   "Failed to create a valid packaged experiment.");
		return false; 		
	}
	shared_ptr<Event> new_experiment(new Event(GlobalSystemEventVariable->getCodecCode(), 
												 experiment));
	putEvent(new_experiment);
	
	return true;
}

bool Server::closeExperiment(){
	if(!GlobalCurrentExperiment){
		return false;
	}
	
	string expName(GlobalCurrentExperiment->getExperimentName());

	putEvent(EventFactory::closeExperimentControl(expName));
	return true;
}

void Server::startDataFileManager() {
    if(GlobalDataFileManager == NULL) {
        GlobalDataFileManager = new DataFileManager();
    }
}

bool Server::openDataFile(const char * path, int options) {
    putEvent(EventFactory::dataFileOpenControl(path, M_OVERWRITE));
    return true;
}

void Server::closeFile() {
    // TODO this could cause some problems.... but since the data file manager
    // is a global this and the event handler doesnt care about the name
    // this will be file for now.
    putEvent(EventFactory::closeDataFileControl(""));
}

bool Server::isDataFileOpen() {
    return GlobalDataFileManager->isFileOpen();
}

bool Server::isExperimentRunning() {
	shared_ptr <StateSystem> state_system = StateSystem::instance();
	return state_system->isRunning();
}

void Server::startExperiment() {
	putEvent(EventFactory::startExperimentControl());        
}

void Server::stopExperiment() {
	putEvent(EventFactory::stopExperimentControl());        
}

MonkeyWorksTime Server::getReferenceTime() {
	shared_ptr <Clock> clock = Clock::instance();
	return clock->getSystemReferenceTime();
}

//shared_ptr<Variable> Server::getVariable(const std::string &tag) {
//	return GlobalVariableRegistry->getVariable(tag);
//}
//
//shared_ptr<Variable> Server::getVariable(const int code) {
//	return GlobalVariableRegistry->getVariable(code);
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
//	return GlobalVariableRegistry->getVariableTagnames();
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

