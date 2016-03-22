/**
 * NetworkConnectionStats.h
 *
 * Description: A container for statistics about a network connection.
 * UPDATE 12/2009: this class does not appear to be in any significant use in 2009
 *
 * Copyright 2005 MIT. All rights reserved.
 */

#ifndef _NETWORK_CONNECTION_STATS_H__
#define _NETWORK_CONNECTION_STATS_H__

#include "MWorksTypes.h"
#include "EventBuffer.h"
#include "Lockable.h"


BEGIN_NAMESPACE_MW


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
        MWTime connectionBorn;
        
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
         NetworkStats getEventsReceivedOfType(int code);
         NetworkStats getEventsTransmittedOfType(int code);
         MWTime getConnectionLifeInUS();
         double getConnectionLifeInS();
         MWTime getConnectionSpawnTime();
         void setConnectionSpawnTime(MWTime bt);
         void eventReceived(int code, unsigned int numBytes);
         void eventTransmitted(int code, unsigned int numBytes);
};


END_NAMESPACE_MW


#endif

