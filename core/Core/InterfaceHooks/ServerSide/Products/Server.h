/*!
 * @header Server.h
 *
 * @discussion 
 *
 * History:<BR/>
 * paul on 1/26/06 - Created.<BR/>
 * 
 * @copyright MIT
 * @updated 1/26/06
 * @version 1.0.0
 */

#ifndef _SERVER_H__
#define _SERVER_H__

#include <boost/filesystem/path.hpp>

#include "EventListener.h"
#include "ZeroMQServer.hpp"
#include "VariableRegistryInterface.h"
#include "RegisteredSingleton.h"


BEGIN_NAMESPACE_MW


class Server : public RegistryAwareEventStreamInterface {
    
    private:
        const boost::shared_ptr<EventBuffer> incoming_event_buffer;
        int listenPort;
        std::string hostname;
        boost::shared_ptr<EventListener> incomingListener;
        boost::shared_ptr<EventListener> outgoingListener;
        std::unique_ptr<ZeroMQServer> server;
    
	public:
        Server();
        ~Server();
        
        /*!
         * @function startServer
         * @discussion Attempts to start the server on
         * whatever port and host it has been set up for.
         *
         * @result True if the server starts properly, false otherwise.
         */
        bool startServer();
    
        /*!
         * @function stopServer
         * @discussion Convenience method that stops accepting clients and
         * shutsdown the server.
         */
        void stopServer();        

        /*!
         * @function saveVariables
         * @discussion Method to write the current variables to a file in the
		 * experiment folder
         */
        void saveVariables(const boost::filesystem::path &file);
        
        /*!
         * @function loadVariables
         * @discussion Method to read a file in the
		 * experiment folder that contains the saved variables
         */
        void loadVariables(const boost::filesystem::path &file);
        
        /*!
         * @function openExperiment
         * @discussion Opens the experiment locaated at the path given as
         * its argument.  It creates an ExperimentPackager object and
         * then uses that to put a system event into the incoming network 
         * stream.  This method is used to load experiments  without using a
         * client.  By putting the event in the incoming network stream though
         * it behaves as if it came from a client so all the proper messages
         * get sent if clients are listening.
         *
         * @param path A path to an experiment.
         * @result True if a system event was sent false otherwise.
         */
        bool openExperiment(const std::string &expPath);
        
		bool closeExperiment();
		
        /*!
         * @function startDataFileManager
         * @discussion Creates an instance of DataFileManager and sets it
         * to the global object. TODO - shouldnt this just make one manager
         * and maybe keep a handle to it?
         */
        void startDataFileManager();
        
        /*!
         * @function openDataFile
         * @discussion Puts an event in the incoming network buffer that
         * will tell the data file manager to open a file at path.  Options
         * is useless at this point because it will always over write 
         * a file.
         *
         * @param path A path where you wish to open a data file.
         * @param options File options to control perhps over writing.  For
         * now though it is ignored.
         * @result Always true. Failure will come in the event sent from the
         * manager.
         */
        void openDataFile(const std::string &path, bool overwrite = false);
        
        bool isDataFileOpen();
		
		bool isExperimentRunning();
		
		void startExperiment();
		
		void stopExperiment();
		
		MWTime getReferenceTime();
		
        void handleEvent(shared_ptr<Event> event) override;
        void putEvent(shared_ptr<Event> event) override;
		
		void setListenPort(int port);
		void setHostname(const std::string &name);
		
		bool isStarted();
		bool isExperimentLoaded();
        
        
        REGISTERED_SINGLETON_CODE_INJECTION( Server )
};


END_NAMESPACE_MW


#endif
