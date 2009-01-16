/**
 * ScarabServerConnection.cpp
 *
 * History:
 * Paul Jankunas on 8/9/05 - Created. 
 *
 * Copyright 2005 MIT. All rights reserved.
 */

#include "ScarabServerConnection.h"
#include <string.h>
using namespace mw;

#define THREAD_INTERVAL_US 200000 

ScarabServerConnection::ScarabServerConnection(shared_ptr<BufferManager> _buffer_manager, int interval) {
    
	buffer_manager = _buffer_manager;
	
	// server connections dont need a uri because they wait for
    // connections using accept
    reader = shared_ptr<ScarabReadConnection>(new ScarabReadConnection(buffer_manager, "empty"));
    writer = shared_ptr<ScarabWriteConnection>(new ScarabWriteConnection(buffer_manager, "empty"));
    reader->setSibling(writer);
    writer->setSibling(reader);
    
	readPort = writePort = -2;
	scheduleInterval = ((interval < 0) ? THREAD_INTERVAL_US : interval);
}

ScarabServerConnection::~ScarabServerConnection() { }

shared_ptr<NetworkReturn>  ScarabServerConnection::accept(ScarabSession * listener) {
    // ScarabClient objects have both a ReadConnection and WriteConnection
    // they connect in the order read first then write.  So the server must
    // accept them in that order.  So the first accepted client will be a 
    // write connection here and a read connection on the other end, and the
    // next connection will be a read connection here and a write connection
    // on the other end.  This round about way of doing things comes about 
    // because the Scarab library is using blocking IO and it would be a really
    // big task to alter the entire Scarab library to do non-blocking IO so
    // this is the easy fix.
    shared_ptr<NetworkReturn>  rc;
    shared_ptr<NetworkReturn>  readRet;
	
	if(writer == NULL || reader == NULL) {
        shared_ptr<NetworkReturn>  rc(new NetworkReturn());
        rc->setMonkeyWorksCode(NR_ERROR);
        rc->setInformation("Read or Write connection is not valid");
        rc->appendInformationWithFormat("Read -(%x)- Write -(%x)-", 
                                                            reader.get(), writer.get());
        return rc;
    }
    rc = writer->accept(listener);
    if(!rc->wasSuccessful()) {
        return rc;
    }
    readRet = reader->accept(listener);
    if(!readRet->wasSuccessful()) {
        return readRet;
    }
    rc->appendInformation(readRet->getInformation());

    return rc;
}

void ScarabServerConnection::start() {
    if(reader) {
        reader->startThread(scheduleInterval);
    }
    if(writer) {
        writer->startThread(scheduleInterval);
    }
}

void ScarabServerConnection::disconnect() {
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
}

int ScarabServerConnection::getReadPort() {
    if(readPort == -2) {
        readPort = reader->getPortNumber(false);
    }
    return readPort;
}

int ScarabServerConnection::getWritePort() {
    if(writePort == -2) {
        writePort = writer->getPortNumber(false);
    }
    return writePort;
}

std::string ScarabServerConnection::getForeignHost() {
    std::string rc;
    if(foreignHost.size() == 0) {
        rc = reader->getHostName(false);
        if(rc.size() == 0) {
            foreignHost = "TBA";
        } else {           
            foreignHost = rc;
        }
    }
    return foreignHost;
}
