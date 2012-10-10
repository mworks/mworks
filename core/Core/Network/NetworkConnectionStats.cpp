/**
 * NetworkConnectionStats.cpp
 *
 * History:
 * Paul Jankunas on 8/5/05 - Created.
 *
 * Copyright 2005 MIT. All rights reserved.
 */

#include "NetworkConnectionStats.h"


BEGIN_NAMESPACE_MW


const double USEC_TO_SEC = 1000000.0;

NetworkConnectionStats::NetworkConnectionStats() {
    numberOfEventsTx = (NetworkStats)0;
    numberOfEventsRx = (NetworkStats)0;
    numberBytesTx = (NetworkStats)0;
    numberBytesRx= (NetworkStats)0;
    eventsTxByType = new NetworkStats[N_RESERVED_CODEC_CODES];
    eventsRxByType = new NetworkStats[N_RESERVED_CODEC_CODES];
    for(int i = 0;  i < (int)N_RESERVED_CODEC_CODES; i++) {
        eventsTxByType[i] = 0;
        eventsRxByType[i] = 0;
    }
    connectionBorn= (MWTime)0;
}

NetworkConnectionStats::~NetworkConnectionStats() {
    if(eventsTxByType) {
        delete [] eventsTxByType;
        eventsTxByType = NULL;
    }
    if(eventsRxByType) {
        delete [] eventsRxByType;
        eventsRxByType = NULL;
    }
}

NetworkStats NetworkConnectionStats::getTotalEventsReceived() {
    SAFE_GET(NetworkStats, numberOfEventsRx);
}

NetworkStats NetworkConnectionStats::getTotalEventsTransmitted() {
    SAFE_GET(NetworkStats, numberOfEventsTx);
}

NetworkStats NetworkConnectionStats::getNumberOfBytesTransmitted() {
    SAFE_GET(NetworkStats, numberBytesTx);
}

NetworkStats NetworkConnectionStats::getNumberOfBytesReceived() {
    SAFE_GET(NetworkStats, numberBytesRx);
}

NetworkStats NetworkConnectionStats::getEventsReceivedOfType(int type) {
    if(type < 0 || type >= N_RESERVED_CODEC_CODES) { 
        return static_cast<NetworkStats>(0);
    }
    M_ISLOCK;
    NetworkStats result = eventsRxByType[type];
    M_ISUNLOCK;
    return result;
}

NetworkStats NetworkConnectionStats::getEventsTransmittedOfType(
                                                            int type) {
    if(type < 0 || type >= N_RESERVED_CODEC_CODES) { 
        return static_cast<NetworkStats>(0);
    }
    M_ISLOCK;
    NetworkStats result = eventsTxByType[type];
    M_ISUNLOCK;
    return result;
}

MWTime NetworkConnectionStats::getConnectionLifeInUS() {
    M_ISLOCK;
    if(connectionBorn == 0) { 
        MWTime result = connectionBorn;
        M_ISUNLOCK;
        return result; 
    }

	shared_ptr <Clock> clock = Clock::instance();
    MWTime current = clock->getCurrentTimeUS();
    MWTime result = (current - connectionBorn);
    M_ISUNLOCK;
    return result;
}

double NetworkConnectionStats::getConnectionLifeInS() {
    M_ISLOCK;
    if(connectionBorn == 0) { 
        M_ISUNLOCK;
        return 0.0;
    }
	
	shared_ptr <Clock> clock = Clock::instance();
    MWTime current = clock->getCurrentTimeUS();
    double result = ((double)(current - connectionBorn)/USEC_TO_SEC);
    M_ISUNLOCK;
    return result;
}

MWTime NetworkConnectionStats::getConnectionSpawnTime() {
    SAFE_GET(MWTime, connectionBorn);
}

void NetworkConnectionStats::setConnectionSpawnTime(MWTime bt) {
    SAFE_SET(connectionBorn, bt);
}

void NetworkConnectionStats::eventReceived(int type,    
                                                    unsigned int numBytes) {
    M_ISLOCK;
    numberOfEventsRx++;
    numberBytesRx += static_cast<NetworkStats>(numBytes);
    if(type >= 0 &&  type < N_RESERVED_CODEC_CODES) {
        eventsRxByType[type]++;
    }
    M_ISUNLOCK;
}

void NetworkConnectionStats::eventTransmitted(int type, 
                                                    unsigned int numBytes) {
    M_ISLOCK;
    numberOfEventsTx++;
    numberBytesTx += static_cast<NetworkStats>(numBytes);
    if(type >= 0 &&  type < N_RESERVED_CODEC_CODES) {
        eventsTxByType[type]++;
    }
    M_ISUNLOCK;
}


END_NAMESPACE_MW
