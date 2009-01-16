/**
 * ScarabClient.cpp
 *
 * History:
 * Paul Jankunas on 11/8/04 - Created.
 * Paul Jankunas on 8/5/05 - Retooling
 *
 * Copyright 2004 MIT. All rights reserved.
 */
 
#include "ScarabClient.h"
#include "EventBuffer.h"
#include "EventFactory.h"
#include <string.h>
#include <boost/format.hpp>
using namespace mw;


#define	USE_TCP_BUFFERED	1

#undef  USE_TCPSELECT
//#define USE_TCPSELECT

#ifdef USE_TCPSELECT
	const char * SCARAB_URI = "ldobinary:tcpselect://";
#else

	#if USE_TCP_BUFFERED
		const char *SCARAB_URI = "ldobinary:tcp_buffered://";
	#else
		const char * SCARAB_URI = "ldobinary:tcp://";
	#endif 
#endif

const int DEFAULT_THREAD_INTERVAL_US = 20000;

ScarabClient::ScarabClient(shared_ptr<BufferManager> _buffer_manager) {
    serverPort = 0;
    threadInterval = DEFAULT_THREAD_INTERVAL_US;
	buffer_manager = _buffer_manager;
}

ScarabClient::ScarabClient(shared_ptr<BufferManager> _buffer_manager, std::string  server, int port) {
    
	buffer_manager = _buffer_manager;
	
	// five is max port length + 1 for colon + 1 for NULL
    std::string tmp = SCARAB_URI + server + ":" + 
										(boost::format("%d")%port).str();

    reader = shared_ptr<ScarabReadConnection>(new ScarabReadConnection(buffer_manager, tmp));
    writer = shared_ptr<ScarabWriteConnection>(new ScarabWriteConnection(buffer_manager, tmp));

	host = server;

    serverPort = port;

    reader->setSibling(writer);
    writer->setSibling(reader);
    threadInterval = DEFAULT_THREAD_INTERVAL_US;
}

ScarabClient::~ScarabClient() { }

int ScarabClient::prepareForConnecting() {
    // make sure things are ok first
    if(host.size() == 0) { return -1; }
    
    //create a proper uri format
    std::string tmp = SCARAB_URI + host + ":" + (boost::format("%d")%serverPort).str();
    
    // create the read and write connections
	if(reader == shared_ptr<ScarabReadConnection>()){
		reader = shared_ptr<ScarabReadConnection>(new ScarabReadConnection(buffer_manager, tmp));
    }
    if(writer == shared_ptr<ScarabWriteConnection>()){
        writer = shared_ptr<ScarabWriteConnection>(new ScarabWriteConnection(buffer_manager, tmp));     
    }
	
    reader->setSibling(writer);
    writer->setSibling(reader);
    
    return 0;
}

shared_ptr<NetworkReturn> ScarabClient::connect() {
    shared_ptr<NetworkReturn> rc;
    if(prepareForConnecting() < 0) {
        rc = shared_ptr<NetworkReturn>(new NetworkReturn());
        rc->setMonkeyWorksCode(NR_FAILED);
        rc->setInformation("Host or Reader or Writer is NULL");
        return rc;
    }
    rc = reader->connect();
    if(!rc->wasSuccessful()) {
        mnetwork("mScarabClient::connect() failed on read connection");
        return rc;
    }
    shared_ptr<NetworkReturn> writeRc;
    writeRc = writer->connect();
    rc->appendInformation(writeRc->getInformation());
    if(!writeRc->wasSuccessful()) {
            mnetwork("mScarabClient::connect() failed on write connection");
            mnetwork("Closing read connection");
            reader->disconnect();
            return rc;
    }
    mnetwork("Incoming network session connected");
    buffer_manager->putEvent(EventFactory::clientConnectedToServerResponse());
    return rc;
}

void ScarabClient::disconnect() {
    if(reader) {
        mdebug("Disconnecting reader %d", reader->getID());
        // the call to disconnect is in the connection class after
        // the service is interrupted
        reader->setInterrupt(true);
    }
    if(writer) {
        mdebug("Disconnecting writer %d", writer->getID());
        // the call to disconnect is in the connection class after
        // the service is interrupted
        writer->setInterrupt(true);
    }
    buffer_manager->putEvent(EventFactory::clientDisconnectedFromServerResponse());
}

void ScarabClient::start() {
    if(reader != 0) {
        reader->startThread(threadInterval);
    }
    if(writer != 0) {
        writer->startThread(threadInterval);
    }
}

std::string  ScarabClient::getServerAddress() {
    return static_cast<std::string >(host);
}

int ScarabClient::getServerPort() {
    return serverPort;
}

bool ScarabClient::isConnected() {
    return ((reader->isConnected()) && (writer->isConnected()));
}

void ScarabClient::setServerAddress(std::string  h) {
    if(h.size()) {
        host = h;
    }
}

void ScarabClient::setServerPort(int p) {
    serverPort = p;
}