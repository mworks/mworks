/**
 * ScarabClient.h
 * 
 * Description:
 *
 * History:
 * Paul Jankunas on 11/8/04 - Created. 
 *
 * Copyright 2004 MIT. All rights reserved.
 */

#ifndef _SCARAB_CLIENT_H__
#define _SCARAB_CLIENT_H__

#include "ScarabConnection.h"
#include "ScarabReadConnection.h"
#include "ScarabWriteConnection.h"
namespace mw {
class ScarabClient {
    protected:
        shared_ptr<ScarabReadConnection> reader;  // a connection for reading
        shared_ptr<ScarabWriteConnection> writer; // a connection for writing
        std::string host; // the host we are accepting on or connecting to
        int serverPort; // the listening port of the server
        int threadInterval; // tweak this for performance
		
		shared_ptr<BufferManager> buffer_manager;
	
    private:
        // prepares the client so that it can call connect
        int prepareForConnecting();
        
    public:
        ScarabClient(shared_ptr<BufferManager> _buffer_manager);
        /**
         * Constructs a client.
         */
        ScarabClient(shared_ptr<BufferManager> _buffer_manager, std::string server, int port);

        /**
         * Destroys a client.
         */
        virtual ~ScarabClient();

        /**
         * Connects to the uri that this object was constructed with.  Does
         * not start the servicing threads.
         */
        shared_ptr<NetworkReturn> connect();

        /**
         * Closes a connection.
         */
        void disconnect();

        /**
         * Starts read and writing on an open connection.
         */
        void start();

        /**
         * 
         */
        std::string  getServerAddress();

        /**
         * 
         */
        int getServerPort();

        /**
         * Is the client connected to something.
         */
        bool isConnected();
        
        void setServerAddress(std::string );
        void setServerPort(int);
};
}
#endif
