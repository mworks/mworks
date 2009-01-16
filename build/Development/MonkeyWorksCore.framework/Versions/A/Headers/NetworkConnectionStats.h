/**
 * NetworkConnectionStats.h
 *
 * Description: A container for statistics about a network connection.
 *
 * History:
 * Paul Jankunas on 8/5/05 - Created.
 * Paul Jankunas on 8/8/05 - Made subclass of Lockable
 *
 * Copyright 2005 MIT. All rights reserved.
 */

#ifndef _NETWORK_CONNECTION_STATS_H__
#define _NETWORK_CONNECTION_STATS_H__

#include "MonkeyWorksTypes.h"
#include "EventBuffer.h"
#include "Lockable.h"
namespace mw {
class NetworkConnectionStats : Lockable {
    protected:
        // number of events transmitted
        NetworkStats numberOfEventsTx;
        // number of events received
        NetworkStats numberOfEventsRx;
        // number of bytes transmitted
        NetworkStats numberBytesTx;
        // number of bytes received.
        NetworkStats numberBytesRx;
        // an array of number of events sent per type
        NetworkStats * eventsTxByType;
        // an array of number of event received by type
        NetworkStats * eventsRxByType;
        // when the connection was started. in usec
        // (connected to for accept connections)
        MonkeyWorksTime connectionBorn;
        
        // disable copy constructor
        NetworkConnectionStats(const NetworkConnectionStats& ) { }
        // disable assignment operator
        void operator=(const NetworkConnectionStats&) { }

    public:
        /**
         * Default constructor. All values zeroed
         */
        NetworkConnectionStats();
        
        /**
         * Frees the arrays.
         */
        ~NetworkConnectionStats();

        /**
         * Below are the interface functions to the statistics class.
         * They are made inline so that they are fast so as not to slow
         * down the network much.  They are all pretty self explanatory.
         * One caveat is the getConnectionLifeInS function returns a double
         * so as to be more compatible with Cocoa times.
         */
         NetworkStats getTotalEventsReceived();
         NetworkStats getTotalEventsTransmitted();
         NetworkStats getNumberOfBytesTransmitted();
         NetworkStats getNumberOfBytesReceived();
         NetworkStats getEventsReceivedOfType(EventCode type);
         NetworkStats getEventsTransmittedOfType(EventCode type);
         MonkeyWorksTime getConnectionLifeInUS();
         double getConnectionLifeInS();
         MonkeyWorksTime getConnectionSpawnTime();
         void setConnectionSpawnTime(MonkeyWorksTime bt);
         void eventReceived(EventCode type, unsigned int numBytes);
         void eventTransmitted(EventCode type, unsigned int numBytes);
};
}
#endif

