/**
 * ScarabServer.h
 * 
 * Description:
 *
 * History:
 * Paul Jankunas on 11/8/04 - Created.
 * Paul Jankunas on 8/5/05 - Removed the accept connection class and moved
 *      functionality into this class.  Added port hopping to the listening
 *      process to avoid bind errors.
 *
 * Copyright 2004 MIT. All rights reserved.
 */

#ifndef _SCARAB_SERVER_H__
#define _SCARAB_SERVER_H__


#include "ScarabServerConnection.h"
#include "NetworkReturn.h"
#include "LinkedList.h"

#include "boost/enable_shared_from_this.hpp"
namespace mw {
// DDC a kludge for linux (why did this even work under OS X? this defined in
// several places)
#ifndef M_IO_MODEL
	#define M_IO_MODEL
	enum IOModel { M_MULTIPLEXING_IO = 0, M_BLOCKING_IO, M_MAX_IO_MODEL };
#endif

// some constants these are used for GUI preferences.
extern const char * const DEFAULT_HOST_NAME;
extern const int DEFAULT_PORT_NUMBER_LOW;
extern const int DEFAULT_PORT_NUMBER_HIGH;
extern const int DEFAULT_MAX_NUMBER_OF_CLIENTS;
extern const IOModel DEFAULT_IO_MODEL;

class ScarabServer : public enable_shared_from_this<ScarabServer> {
    protected:
        ScarabSession * listeningSocket; // listen socket
        std::string listenUri; // scarab uri
        int listenPort; // numeric listen port
        std::string listenAddress; // the uri without the scarab stuff
        bool listening; // is the listener started
        bool accepting; // are we accepting clients
        ScarabServerConnection ** clients; // clients connected to the server
        int numberOfConnectedClients;
        int lowServerPort; // low server listen port
        int highServerPort; // high server listen port
        int numberChecked; // how many ports we have tried on.
        int ioModel; // the iomodel
        shared_ptr<ScheduleTask> acceptThread; // a thread to accept connections
        Lockable * connectionLock; // lock for connection access
        Lockable * boolLock;
        int clientThreadInterval; 
        int maxConnections;
        shared_ptr<BufferManager> buffer_manager;
        
    private:
        ScarabServerConnection * tempClient; // created for accepting
        // Takes the listen address and listen port and listen uri
        // and concatenates them.
        std::string createScarabURI();
        // chooses a new port for the server to attempt to listen on
        bool chooseNewPort();
        // initialize stuff to default
        void init();
        // schedules the accept thread
        void scheduleAccept();
        
    public:

        /**
         * Standard constructor sets member values to default values and
         * sets the listener address to host and listen port number to
         * portNumber.
         */
        ScarabServer(shared_ptr<BufferManager> _buffer_manager, 
                             std::string host = DEFAULT_HOST_NAME, 
                             int startPort = DEFAULT_PORT_NUMBER_LOW, 
                             int lowPort = DEFAULT_PORT_NUMBER_LOW, 
                             int highPort = DEFAULT_PORT_NUMBER_LOW);

        /**
         * Standard destructor.
         */
        virtual ~ScarabServer();
        
        /**
         * Starts a scarab listening session.
         */
        NetworkReturn * startListening();

        /**
         * Starts a thread where the server will wait for connections to
         * accept.  New connections are add to the list of clients and then
         * the server calls accept again.  The new clients are automatically
         * started by the server.  The interval argument tells the scheduler
         * how often to run the thread.
         */
        NetworkReturn * startAccepting();

        /**
         * Causes the execution of the accepting thread to stop.  The listening
         * socket still remains active, so another call to startAccepting
         * will start the thread back up.
         */
        void stopAccepting();

        /**
         * Stops the accepting thread if it is running and closes the 
         * listening connection.
         */
        void shutdown();

        /**
         * Is the server running.
         */
        bool isConnected() { return listening; }
        
        /**
         * Has the accept loop started.
         */
        bool isAccepting();
        
        /**
         * Is the accept thread active?
         */
        bool isActive();
        
        /**
         * Disconnects a client from the server.
         */
        void disconnectClient(int cliNum);

        /**
         * Service the listening socket to accept connections.
         */
        int service();

        /***
         Accessor/Mutator Methods
         ***/
        void setServerHostname(std::string);
        void setMaximumConnections(int);
        void setIOModel(IOModel);
        void setServerListenLowPort(int);
        void setServerListenHighPort(int);
        void setThreadScheduleInterval(int interval);
        
        int getMaxNumberOfConnections();
        int getClientReadPort(int);
        int getClientWritePort(int);
        int getServerListeningPort();
        int getNumberOfClients();
        std::string getServerHostAddress();
        std::string getClientAddress(int);
        int getDefaultLowPort();
        int getDefaultHighPort();
};
}
#endif 
