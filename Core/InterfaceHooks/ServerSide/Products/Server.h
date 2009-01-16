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

#include "IncomingEventListener.h"
#include "OutgoingEventListener.h"
#include "GenericVariable.h"
#include <boost/thread/recursive_mutex.hpp>
#include <boost/filesystem/path.hpp>
#include "GenericEventFunctor.h"
#include "ScarabServer.h"
namespace mw {
class Server : public EventHandler {
protected:
	shared_ptr<ScarabServer> server;
	shared_ptr<OutgoingEventListener> outgoingListener;
	shared_ptr<IncomingEventListener> incomingListener;
	std::vector <std::vector<shared_ptr<GenericEventFunctor> > > callbacks;
	
	//boost::mutex event_lock;
	boost::recursive_mutex callbacksLock;
	
	void updateCallbacks(const unsigned int vars);
    public:
        Server();
	virtual ~Server();
        
        /*!
         * @function startServer
         * @discussion Attempts to start its ScarabServer object on
         * whatever port and host it has been set up for.  To change the
         * server options you can use the Server::getScarabServer method
         * to get a handle to the server and change its options.
         *
         * @result True if the server starts properly, false otherwise.
         * Extended error information can be found in the lastNetworkReturn.
         * TODO make accessor.
         */
        bool startServer();
        
        /*!
         * @function startAccepting
         * @discussion convenience method to make the server start accepting
         * clients again in case you called stop accepting.
         */
        void startAccepting();
        
        /*!
         * @function stopAccepting
         * @discussion Convenience method to stop a server from accepting 
         * clients.
         */
        void stopAccepting();
        
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
        bool openDataFile(const char * path, int options = 0);
        
        /*!
         * @function closeFile
         * @discussion Closes a data file if one is open.  If it closed
         * a file a message is sent by the data file manager.
         */
        void closeFile();
        
        bool isDataFileOpen();
		
		bool isExperimentRunning();
		
		void startExperiment();
		
		void stopExperiment();
		
		MonkeyWorksTime getReferenceTime();
		shared_ptr<Variable> getVariable(const std::string &tag);
		shared_ptr<Variable> getVariable(const int code);
		int getCode(const std::string &tag);
		std::vector<std::string> getVariableNames();
		void handleEvent(shared_ptr<Event> &event);
		void registerCallback(shared_ptr<GenericEventFunctor> gef);
		void registerCallback(shared_ptr<GenericEventFunctor> gef, const int code);
		void unregisterCallbacks(const std::string &key);
		
		void setListenLowPort(const int port);
		void setListenHighPort(const int port);
		void setHostname(const std::string &name);
		
		bool isStarted();
		bool isAccepting();
		bool isExperimentLoaded();
};
}
#endif
