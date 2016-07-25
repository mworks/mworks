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


BEGIN_NAMESPACE_MW


class ScarabClient {
    private:
        shared_ptr<ScarabReadConnection> reader;  // a connection for reading
        shared_ptr<ScarabWriteConnection> writer; // a connection for writing
        std::string host; // the host we are accepting on or connecting to
        int serverPort; // the listening port of the server
        int threadInterval; // tweak this for performance
		
		shared_ptr<EventBuffer> incoming_event_buffer;
        shared_ptr<EventBuffer> outgoing_event_buffer;
        
    public:
        /**
         * Constructs a client.
         */
    ScarabClient(shared_ptr<EventBuffer> _incoming_event_buffer, 
                 shared_ptr<EventBuffer> _outgoing_event_buffer, 
                 std::string server, int port);

        /**
         * Connects to the uri that this object was constructed with and
         * starts the servicing threads.
         */
        bool connect();

        /**
         * Closes a connection.
         */
        void disconnect();

        /**
         * Is the client connected to something.
         */
        bool isConnected();
};


END_NAMESPACE_MW


#endif
