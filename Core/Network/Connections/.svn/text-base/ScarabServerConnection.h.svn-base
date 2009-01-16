/**
 * ScarabServerConnection.h
 * 
 * Description: Used by the Scarab Server to accept client connections.  
 *  Moved the functionality for accepting clients from the ScarabClient class
 *  to here because i was confusing myself.
 *
 * History:
 * Paul Jankunas on 8/9/05 - Created.
 * Paul Jankunas on 08/24/05 - Added new member scheduleInterval to control
 *     thread scheduling
 *
 * Copyright 2005 MIT. All rights reserved.
 */

#ifndef _SCARAB_SERVER_CONNECTION_H__
#define _SCARAB_SERVER_CONNECTION_H__

#include "ScarabReadConnection.h"
#include "ScarabWriteConnection.h"
#include "NetworkReturn.h"
namespace mw {
class ScarabServerConnection {
    protected:
        shared_ptr<ScarabReadConnection> reader;  // a connection for reading
        shared_ptr<ScarabWriteConnection> writer; // a connection for writing
        int readPort; // readPort we have connected to
        int writePort; // writePort we have connected to
        std::string foreignHost;
        int scheduleInterval;
		
		shared_ptr<BufferManager> buffer_manager;

    public:
        
        /**
         * Initializes the object.  Interval must be a positive time.  It
         * is the interval that the read and write threads are scheduled at.
         * A negative value will result in the default value of 200ms 
         */
        ScarabServerConnection(shared_ptr<BufferManager> _buffer_manager, int interval);
        
        /**
         * Frees memory.
         */
        ~ScarabServerConnection();
        
        /**
         * Wait for incoming connections.  Call blocks.
         */
        shared_ptr<NetworkReturn>  accept(ScarabSession * listener);
        
        /**
         * Starts the read write threads
         */
        void start();
        
        /**
         * Disconnects the client attached to this connection.
         */
        void disconnect();
        
        /**
         * Returns the ports that are used for reading and writing.
         */
        int getReadPort();
        int getWritePort();
        
        /**
         * Returns the foreign hostname.
         */
        std::string getForeignHost();
};
}
#endif
