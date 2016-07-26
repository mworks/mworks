/**
 * ScarabServer.cpp
 *
 * History:
 * Paul Jankunas on 11/8/04 - Created.
 *
 * Copyright 2004 MIT. All rights reserved.
 */

#include "ScarabServer.h"
#include "ScarabServices.h"
#include "EventBuffer.h"
#include "SystemEventFactory.h"

#include "Event.h"
#include <string>
#include "boost/bind.hpp"
#include "DataFileManager.h"
#include "NetworkReturn.h"


BEGIN_NAMESPACE_MW


	const MWTime INITIAL_THREAD_DELAY = 0;
	const MWTime REPEAT_INTERVAL = 20;
	const int TIMES_TO_RUN = 1;
	
	
#define	USE_TCP_BUFFERED	1
	
#undef  USE_TCPSELECT
	//#define USE_TCPSELECT
	
#ifdef USE_TCPSELECT
	const char * DEFAULT_SCARAB_URI = "ldobinary:tcpselect://";
#else
	
#ifdef USE_TCP_BUFFERED
	const char *DEFAULT_SCARAB_URI = "ldobinary:tcp_buffered://";
#else
	const char * DEFAULT_SCARAB_URI = "ldobinary:tcp://";
#endif 
#endif
	
	// short circuit
#define SCARAB_TCP_URI	DEFAULT_SCARAB_URI
#define SCARAB_SELECT_URI	DEFAULT_SCARAB_URI
	
	//#define FUDGE_HOST_NAME
	
#ifdef FUDGE_HOST_NAME
	const char * const DEFAULT_HOST_NAME = "10.170.2.37";
#else 
	const char * const DEFAULT_HOST_NAME =  "127.0.0.1";
#endif
	
	const int DEFAULT_PORT_NUMBER_LOW = 19989;
	const int DEFAULT_PORT_NUMBER_HIGH = 19999;
	const int DEFAULT_MAX_NUMBER_OF_CLIENTS = 16;


ScarabServer::ScarabServer(shared_ptr<EventBuffer> _incoming_event_buffer, 
                           shared_ptr<EventBuffer> _outgoing_event_buffer, 
                             std::string host, 
                             int startPort, 
                             int lowPort, 
                             int highPort){
    init();
    incoming_event_buffer = _incoming_event_buffer;
    outgoing_event_buffer = _outgoing_event_buffer;
    listenPort = startPort;
    lowServerPort = lowPort;
    highServerPort = highPort;
    // copy the host address
	listenAddress = host;
}

void ScarabServer::init() {
    // set default values for stuff
    listeningSocket = NULL;
    listening = false;
    accepting = false;
    clients = new ScarabServerConnection *[DEFAULT_MAX_NUMBER_OF_CLIENTS];
    numberOfConnectedClients = 0;
    lowServerPort = DEFAULT_PORT_NUMBER_LOW;
    highServerPort = DEFAULT_PORT_NUMBER_HIGH;
    numberChecked = 0;
    connectionLock = new Lockable();
    boolLock = new Lockable();
    clientThreadInterval = -1; // makes connection use default
    maxConnections = DEFAULT_MAX_NUMBER_OF_CLIENTS;
	
    listenUri = DEFAULT_SCARAB_URI;
	
}

ScarabServer::~ScarabServer() {
	
    if(clients) {
        delete  [] clients;
        clients = NULL;
    }
    if(listeningSocket) {
        scarab_session_close(listeningSocket);
		listeningSocket = NULL;
    }
    
    if(connectionLock) {
        delete connectionLock;
        connectionLock = NULL;
    }
    if(boolLock) {
        delete boolLock;
        boolLock = NULL;
    }
	
}

std::string ScarabServer::createScarabURI() {
    
	std::string fullPath;
	char * portNumStr;
	
    // convert the portNumber into a string
    portNumStr = new char[6]; // six is all we need at max 65535 + NULL
    snprintf(portNumStr, 6, "%d", listenPort);
    
	
    fullPath = listenUri + listenAddress + ":"  + portNumStr;
	
    delete [] portNumStr;
    return fullPath;
}

bool ScarabServer::chooseNewPort() {
    if((listenPort >= lowServerPort) && (listenPort <= highServerPort)) {
        //listen port was in range
        numberChecked++;
        listenPort++;
        int range = (highServerPort - lowServerPort);
        if(listenPort > highServerPort) {
            listenPort = lowServerPort;
        }
        if(numberChecked > range) {
            listenPort = -1;
            return false;
        } else {
            return true;
        }
    } else {
        // the number passed to the original server was not in the range
        listenPort = lowServerPort;
        return true;
    }
}

void ScarabServer::scheduleAccept() {
	shared_ptr<ScarabServer> this_one = shared_from_this();
	
	shared_ptr<Scheduler> scheduler = Scheduler::instance();
	
    acceptThread = scheduler->scheduleMS(FILELINE,
										 INITIAL_THREAD_DELAY,
										 REPEAT_INTERVAL,
										 TIMES_TO_RUN, 
										 boost::bind(acceptClients, this_one),
										 M_DEFAULT_NETWORK_PRIORITY, 
										 (MWTime)0, // no warnings 
										 M_DEFAULT_NETWORK_FAIL_SLOP_MS,
										 M_MISSED_EXECUTION_DROP);
}

bool ScarabServer::startListening() {
    mdebug("startListening()");
    if(listening) {
        return true;
    }
    
	if(listenUri.size() == 0) {
        return false;
    }
    
	std::string fullUri = createScarabURI();
    if(fullUri.size() == 0) {
        return false;
    }
    int error = 1;
	//    mnetwork("Trying listening socket at %s on port %d",
	//                                                listenAddress.c_str(), 
	//												listenPort);
	//    mprintf("URI = %s", fullUri.c_str());
    listeningSocket = scarab_session_listen(fullUri.c_str());
    error = getScarabError(listeningSocket);
    while(error) {
        mnetwork("Failed to open a listening socket at %s on port %d",
				 listenAddress.c_str(), 
				 listenPort);
		
		//mdebug("Maybe print out why here??");
        // if there is another available port we will try again
        if(!chooseNewPort()) {
            return false;
        }
        fullUri = createScarabURI();
        listeningSocket = scarab_session_listen(fullUri.c_str());
        error = getScarabError(listeningSocket);
    }
    mnetwork("Listening socket started at %s on port %d", listenAddress.c_str(),
			 listenPort);
    listening = true;
    return true;
}

bool ScarabServer::startAccepting() {
    Locker locker(*boolLock);
    if(accepting) {
        return true;
    }
    if(listening) {
        scheduleAccept();
        accepting = true;
    }
    return accepting;
}

void ScarabServer::stopAccepting() {
    boolLock->lock();
    accepting = false;
    boolLock->unlock();
}

void ScarabServer::shutdown() {
    // shut down the accept thread.
    stopAccepting();
    if(acceptThread) {
        acceptThread->cancel();
    }
    if(tempClient) {
        delete tempClient;
        tempClient = NULL;
    }
    // disconnect every client.
    for(int i = 0; i < numberOfConnectedClients; i++) {
        disconnectClient(i);
    }
    if(listening) {
        scarab_session_close(listeningSocket);
        listening = false;
        listeningSocket = NULL;
    }
}

bool ScarabServer::isAccepting() { 
    bool rc;
    boolLock->lock();
    rc = accepting;
    boolLock->unlock();
    return rc;
}

void ScarabServer::disconnectClient(int cliNum) {
    M_HASLOCK(connectionLock);
    if(cliNum >= numberOfConnectedClients) { return; }
    if(cliNum < 0) { return; }
    if(numberOfConnectedClients == 0) { return; }
    clients[cliNum]->disconnect();
    M_HASUNLOCK(connectionLock);
    outgoing_event_buffer->putEvent(SystemEventFactory::serverDisconnectClientResponse());
}


void ScarabServer::setServerHostname(std::string newhost) {
	listenAddress = newhost;
}

void ScarabServer::setServerListenLowPort(int newlow) {
    lowServerPort = newlow;
}

void ScarabServer::setServerListenHighPort(int newhigh) {
    highServerPort = newhigh;
}


void * ScarabServer::acceptClients(const shared_ptr<ScarabServer> &ss) {
    int rc = ss->service();
    if(rc < 0) {
        ss->stopAccepting();
    }
    return NULL;
}


int ScarabServer::service() {
    if(1){
		//while(1) {
        if(getScarabError(listeningSocket)) {
            merror(M_NETWORK_MESSAGE_DOMAIN, 
				   "Session failure on listening socket");
            logDescriptiveScarabMessage(listeningSocket);
            return -1;
        }
        // this number should eventually come from the network
        // manager at construction time.
        connectionLock->lock();
        int check = numberOfConnectedClients;
        connectionLock->unlock();
        if(check == maxConnections) {
            mwarning(M_NETWORK_MESSAGE_DOMAIN,
					 "Maximum number of clients connected");
            return -1;
        }
        // the clients here are accept clients who wait for
        // connection clients to connect to them.
        tempClient = new ScarabServerConnection(incoming_event_buffer, outgoing_event_buffer, clientThreadInterval);
        shared_ptr<NetworkReturn> acceptRet;
        
        acceptRet = tempClient->accept(listeningSocket);
        mprintf("Accepting remote client");
        if(!acceptRet->wasSuccessful()) {
            // the accept failed but it may have just timed out
            delete tempClient;
            tempClient = NULL;
            if(scarab_did_select_timeout(acceptRet->getPackageCode())) {
                // return from the service function and reschedule it
                boolLock->lock();
                if(accepting) {
                    boolLock->unlock();
					return 1;
					//continue;
                } else {
                    boolLock->unlock();
					return 1;
                }
            } else {
                merror(M_NETWORK_MESSAGE_DOMAIN, 
					   "Server failed to accept client");
				return -1;
            }
        }
        // the connection was established.
        mnetwork("Connection accepted from ...");
        tempClient->start();
        M_HASLOCK(connectionLock);
        clients[numberOfConnectedClients] = tempClient;
        numberOfConnectedClients++;
        M_HASUNLOCK(connectionLock);
        outgoing_event_buffer->putEvent(SystemEventFactory::serverConnectedClientResponse());
        
	}
	
	
	
	// Some omissions by Paul:
	// Need to reschedule the accept thread (otherwise, this will be the last
	// client we accept).  Also, we need to startListening again (which seems
	// like it probably ought to have been rolled into accepting, but that
	// is a another mess to clean up on another day...
	startListening();
	scheduleAccept();
	
	return 0;
} 


END_NAMESPACE_MW
