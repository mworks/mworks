/**
 * NetworkConnectionStats.cpp
 *
 * History:
 * Paul Jankunas on 8/5/05 - Created.
 *
 * Copyright 2005 MIT. All rights reserved.
 */

#include "NetworkConnectionStats.h"
using namespace mw;

const double USEC_TO_SEC = 1000000.0;

NetworkConnectionStats::NetworkConnectionStats() {
    numberOfEventsTx = (NetworkStats)0;
    numberOfEventsRx = (NetworkStats)0;
    numberBytesTx = (NetworkStats)0;
    numberBytesRx= (NetworkStats)0;
    eventsTxByType = new NetworkStats[M_MAX_RESERVED_EVENT_CODE];
    eventsRxByType = new NetworkStats[M_MAX_RESERVED_EVENT_CODE];
    for(int i = 0;  i < (int)M_MAX_RESERVED_EVENT_CODE; i++) {
        eventsTxByType[i] = 0;
        eventsRxByType[i] = 0;
    }
    connectionBorn= (MonkeyWorksTime)0;
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

NetworkStats NetworkConnectionStats::getEventsReceivedOfType(EventCode type) {
    if(type < 0 || type >= M_MAX_RESERVED_EVENT_CODE) { 
        return static_cast<NetworkStats>(0);
    }
    M_ISLOCK;
    NetworkStats result = eventsRxByType[type];
    M_ISUNLOCK;
    return result;
}

NetworkStats NetworkConnectionStats::getEventsTransmittedOfType(
                                                            EventCode type) {
    if(type < 0 || type >= M_MAX_RESERVED_EVENT_CODE) { 
        return static_cast<NetworkStats>(0);
    }
    M_ISLOCK;
    NetworkStats result = eventsTxByType[type];
    M_ISUNLOCK;
    return result;
}

MonkeyWorksTime NetworkConnectionStats::getConnectionLifeInUS() {
    M_ISLOCK;
    if(connectionBorn == 0) { 
        MonkeyWorksTime result = connectionBorn;
        M_ISUNLOCK;
        return result; 
    }

	shared_ptr <Clock> clock = Clock::instance();
    MonkeyWorksTime current = clock->getCurrentTimeUS();
    MonkeyWorksTime result = (current - connectionBorn);
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
    MonkeyWorksTime current = clock->getCurrentTimeUS();
    double result = ((double)(current - connectionBorn)/USEC_TO_SEC);
    M_ISUNLOCK;
    return result;
}

MonkeyWorksTime NetworkConnectionStats::getConnectionSpawnTime() {
    SAFE_GET(MonkeyWorksTime, connectionBorn);
}

void NetworkConnectionStats::setConnectionSpawnTime(MonkeyWorksTime bt) {
    SAFE_SET(connectionBorn, bt);
}

void NetworkConnectionStats::eventReceived(EventCode type,    
                                                    unsigned int numBytes) {
    M_ISLOCK;
    numberOfEventsRx++;
    numberBytesRx += static_cast<NetworkStats>(numBytes);
    if(type >= 0 &&  type < M_MAX_RESERVED_EVENT_CODE) {
        eventsRxByType[type]++;
    }
    M_ISUNLOCK;
}

void NetworkConnectionStats::eventTransmitted(EventCode type, 
                                                    unsigned int numBytes) {
    M_ISLOCK;
    numberOfEventsTx++;
    numberBytesTx += static_cast<NetworkStats>(numBytes);
    if(type >= 0 &&  type < M_MAX_RESERVED_EVENT_CODE) {
        eventsTxByType[type]++;
    }
    M_ISUNLOCK;
}
