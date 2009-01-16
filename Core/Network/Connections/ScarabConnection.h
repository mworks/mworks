/**
 * ScarabConnection.h
 *
 * Description:  A container to hold a Scarab session and its memory context.
 * The session can not be a listening socket.  It is allowed only to connect.
 *
 * History:
 * Paul Jankunas on 11/15/04 - Created.
 * 
 * Copyright 2004 MIT. All rights reserved.
 */
 
#ifndef _SCARAB_CONNECTION_H__
#define _SCARAB_CONNECTION_H__

#include <stdlib.h>
#include "Scarab/scarab.h"
#include "Utilities.h"

#include "Lockable.h"
#include "NetworkConnectionStats.h"
#include "NetworkReturn.h"
#include "Scheduler.h"

#include "boost/enable_shared_from_this.hpp"
namespace mw {
class ScarabConnection : public enable_shared_from_this<ScarabConnection> {
    protected:
	boost::mutex connectLock;
	boost::mutex interruptLock;
	ScarabSession * pipe;
	shared_ptr<ScarabConnection> sibling; // allows two connections to be linked
	std::string uri; // the address of the connection
	bool connected; // status
	bool connecting; // status
	bool servicing; // are we servicing this port
	bool interrupt; // interrupt the servicing routine to disconnect
	bool term; // true to terminate the connection
	shared_ptr<ScheduleTask> thread;
	shared_ptr<ScheduleTask> terminateThread;
	long cid; // connection ID
	shared_ptr<NetworkConnectionStats> connectionStats;

	shared_ptr<BufferManager> buffer_manager;

	// disable copying and assignment and default constructor
	ScarabConnection(ScarabConnection& ref) { }
	void operator=(ScarabConnection& ref) { }
	ScarabConnection() { }

    public:
        
        /**
         * Copies the uri and preps the object for connection.
         */
        ScarabConnection(shared_ptr<BufferManager> _buffer_manager, std::string uri);
              
        /**
         * Cleans up the memory for a connection.
         */
        virtual ~ScarabConnection();

        /**
         * Waits for a connection by calling accept().
         */
        shared_ptr<NetworkReturn> accept(ScarabSession * listener);

        /**
         * Creates and establishes the underlying socket connection.
         */
        shared_ptr<NetworkReturn> connect();

        /**
         * Stops the servicing thread
         */
        virtual void stopThread();

        /**
         * Closes the connection
         */
        void disconnect();
        
        /**
         * Allows two connections to communicate with one another.
         */
        void setSibling(shared_ptr<ScarabConnection> sib) { sibling = sib; }
        
        /**
         * Is the session connected. True is connected false otherwise. 
         */
        bool isConnected() { return connected; }
        
        /**
         * Can we terminate this connection?
         */
        bool canTerminate() { return term; }
        
        /**
         * Used by the scheduler to terminate a thread.  It is unsafe to
         * call this function unless you are the function that has been
         * scheduled to call it.  If you call this in any other place
         * you have the possibility of leaving open connections.
         */
        void kill();
        
        /**
         * Tell the connection to stop processing and send a termination
         * sequence.
         */
        void setInterrupt(bool _interrupt) {
			boost::mutex::scoped_lock lock(interruptLock);
			interrupt = _interrupt; 
		}
        
        /**
         * Returns the hostname of this scarab session.  If 'local' is true
         * then the function returns the local hostname, else it will
         * return the foreign hostname, i.e the host it is connected to.
         * The local value would be false if a client wants to know the
         * hostname of the server it is connected to, or if the server wants
         * to inquire about who is connected to it.
         */
        char * getHostName(bool local);
        
        /**
         * Returns the port number that the session is on.  Operates on the
         * same basis as getHostName.
         */
        int getPortNumber(bool local);
        
        /**
         * Returns the ID of the connection, a lame random number
         */
        int getID() { return cid; }
        
    public:
        // All these functions are shells.  The real work is done by the 
        // subclasses.
        /**
         * Starts the servicing thread
         */
        virtual void startThread(int interval);
        
        /**
         * Services the thread.
         */
        virtual int service();
};
}
#endif
 
